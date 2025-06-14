#include "ProcessManagement.hpp"
#include <iostream>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include "../encryptDecrypt/Cryption.hpp"
#include "Task.hpp"
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <atomic>
#include <signal.h>

ProcessManagement::ProcessManagement() : numWorkers(4) {
    fullSemaphore = sem_open("/full_semaphore", O_CREAT, 0666, 0);
    emptySemaphore = sem_open("/empty_semaphore", O_CREAT, 0666, 1000);
    
    if (fullSemaphore == SEM_FAILED || emptySemaphore == SEM_FAILED) {
        std::cerr << "Failed to create semaphores" << std::endl;
        exit(1);
    }
    
    // Initialize shared memory
    shmFd = shm_open(shmName, O_CREAT | O_RDWR, 0666);
    if (shmFd == -1) {
        std::cerr << "Failed to create shared memory" << std::endl;
        exit(1);
    }
    
    ftruncate(shmFd, sizeof(SharedMemory));
    sharedMem = static_cast<SharedMemory*>(mmap(nullptr, sizeof(SharedMemory), 
                                               PROT_READ | PROT_WRITE, MAP_SHARED, shmFd, 0));
    
    if (sharedMem == MAP_FAILED) {
        std::cerr << "Failed to map shared memory" << std::endl;
        exit(1);
    }
    
    sharedMem->front = 0;
    sharedMem->rear = 0;
    sharedMem->size.store(0);
    sharedMem->shutdown.store(false);
    
    // Create worker processes
    createWorkerPool();
}

void ProcessManagement::createWorkerPool() {
    for (int i = 0; i < numWorkers; i++) {
        pid_t pid = fork();
        
        if (pid < 0) {
            std::cerr << "Fork failed for worker " << i << std::endl;
            exit(1);
        } 
        else if (pid == 0) {
            // Child process - become a worker
            workerProcess();
            exit(0);
        } 
        else {
            // Parent process - store worker PID
            workerPids.push_back(pid);
            std::cout << "Created worker process " << pid << std::endl;
        }
    }
}

void ProcessManagement::workerProcess() {
    std::cout << "Worker process " << getpid() << " started" << std::endl;
    
    while (true) {
        if (sharedMem->shutdown.load()) {
            std::cout << "Worker process " << getpid() << " shutting down" << std::endl;
            break;
        }
        if (sem_wait(fullSemaphore) == -1) {
            if (errno == EINTR) continue; 
            break;
        }
        
        if (sharedMem->shutdown.load()) {
            sem_post(fullSemaphore); 
            break;
        }
    
        char taskStr[256];
        bool taskRetrieved = false;
        
        {
            std::unique_lock<std::mutex> lock(queuelock);
            if (sharedMem->size.load() > 0) {
                strcpy(taskStr, sharedMem->tasks[sharedMem->front]);
                sharedMem->front = (sharedMem->front + 1) % 1000;
                sharedMem->size.fetch_sub(1);
                taskRetrieved = true;
            }
        }
        
        if (taskRetrieved) {
            sem_post(emptySemaphore);
            
            std::cout << "Worker " << getpid() << " executing task: " << taskStr << std::endl;
            executeCryption(taskStr);
            std::cout << "Worker " << getpid() << " completed task" << std::endl;
        }
    }
}

bool ProcessManagement::submitToQueue(std::unique_ptr<Task> task) {
    if (sem_wait(emptySemaphore) == -1) {
        std::cerr << "Failed to wait on empty semaphore" << std::endl;
        return false;
    }
    
    {
        std::unique_lock<std::mutex> lock(queuelock);
        
        if (sharedMem->size.load() >= 1000) {
            std::cerr << "Queue is full, cannot submit task." << std::endl;
            sem_post(emptySemaphore); // Put the semaphore back
            return false;
        }
        
        strcpy(sharedMem->tasks[sharedMem->rear], task->toString().c_str());
        sharedMem->rear = (sharedMem->rear + 1) % 1000;
        sharedMem->size.fetch_add(1);
    }
    sem_post(fullSemaphore);
    
    std::cout << "Task submitted to queue. Queue size: " << sharedMem->size.load() << std::endl;
    return true;
}


void ProcessManagement::shutdown() {
    std::cout << "Shutting down process management..." << std::endl;
    
    sharedMem->shutdown.store(true);
    
    for (int i = 0; i < numWorkers; i++) {
        sem_post(fullSemaphore);
    }
    for (pid_t pid : workerPids) {
        int status;
        std::cout << "Waiting for worker process " << pid << " to terminate..." << std::endl;
        waitpid(pid, &status, 0);
        std::cout << "Worker process " << pid << " terminated" << std::endl;
    }
    
    workerPids.clear();
}

ProcessManagement::~ProcessManagement() {
    shutdown();
    
    if (fullSemaphore != SEM_FAILED) {
        sem_close(fullSemaphore);
        sem_unlink("/full_semaphore");
    }
    
    if (emptySemaphore != SEM_FAILED) {
        sem_close(emptySemaphore);
        sem_unlink("/empty_semaphore");
    }
    
    if (sharedMem != MAP_FAILED) {
        munmap(sharedMem, sizeof(SharedMemory));
    }
    
    if (shmFd != -1) {
        close(shmFd);
        shm_unlink(shmName);
    }
}