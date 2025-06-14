#pragma once
#include <memory>
#include <mutex>
#include <vector>
#include <semaphore.h>
#include <atomic>
#include <sys/types.h>

class Task;

struct SharedMemory {
    char tasks[1000][256];
    int front;
    int rear;
    std::atomic<int> size;
    std::atomic<bool> shutdown;
};

class ProcessManagement {
private:
    static constexpr const char* shmName = "/task_queue_shm";
    int shmFd;
    SharedMemory* sharedMem;
    std::mutex queuelock;
    
    // Semaphores
    sem_t* fullSemaphore;
    sem_t* emptySemaphore;
    
    // Worker management
    int numWorkers;
    std::vector<pid_t> workerPids;
    
    void createWorkerPool();
    void workerProcess();
    void shutdown();

public:
    ProcessManagement();
    ~ProcessManagement();
    
    bool submitToQueue(std::unique_ptr<Task> task);
};