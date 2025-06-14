#include <iostream>
#include <filesystem>
#include <thread>
#include <chrono>
#include "./src/app/processes/ProcessManagement.hpp"
#include "./src/app/processes/Task.hpp"


namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    std::string directory;
    std::string action;

    std::cout << "Enter the directory path: " << std::endl;
    std::getline(std::cin, directory);

    std::cout << "Enter the action (ENCRYPT/DECRYPT): " << std::endl;
    std::getline(std::cin, action);

    // Validate action input
    if (action != "ENCRYPT" && action != "DECRYPT") {
        std::cout << "Invalid action. Please enter ENCRYPT or DECRYPT." << std::endl;
        return 1;
    }

    try {
        if (fs::exists(directory) && fs::is_directory(directory)) {
            ProcessManagement processManagement;
            
            std::cout << "Processing directory: " << directory << std::endl;
            std::cout << "Action: " << action << std::endl;
            
            int totalFiles = 0;
            int submittedTasks = 0;
            for (const auto& entry : fs::recursive_directory_iterator(directory)) {
                if (entry.is_regular_file()) {
                    totalFiles++;
                    std::string filePath = entry.path().string();
                    
                    try {
                        IO io(filePath);
                        std::fstream f_stream = io.getFileStream();
                        
                        if (f_stream.is_open()) {
                            Action taskAction = (action == "ENCRYPT") ? Action::ENCRYPT : Action::DECRYPT;
                            auto task = std::make_unique<Task>(std::move(f_stream), taskAction, filePath);
                            
                            if (processManagement.submitToQueue(std::move(task))) {
                                submittedTasks++;
                                std::cout << "Submitted task for file: " << filePath << std::endl;
                            } else {
                                std::cout << "Failed to submit task for file: " << filePath << std::endl;
                            }
                        } else {
                            std::cout << "Unable to open the file: " << filePath << std::endl;
                        }
                    } catch (const std::exception& ex) {
                        std::cout << "Error processing file " << filePath << ": " << ex.what() << std::endl;
                    }
                }
            }
            
            std::cout << "\nSummary:" << std::endl;
            std::cout << "Total files found: " << totalFiles << std::endl;
            std::cout << "Tasks submitted: " << submittedTasks << std::endl;
            
            if (submittedTasks > 0) {
                std::cout << "\nTasks have been submitted to worker processes." << std::endl;
                std::cout << "Workers are processing tasks automatically..." << std::endl;
                
                std::cout << "Waiting for all tasks to complete..." << std::endl;
                
                bool tasksRemaining = true;
                int dotCount = 0;
                
                while (tasksRemaining) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(500));
                    
                    // Print progress dots
                    std::cout << ".";
                    std::cout.flush();
                    dotCount++;
                    
                    if (dotCount % 10 == 0) {
                        std::cout << std::endl;
                    }
                    static int maxWaitCycles = 120;
                    static int waitCycles = 0;
                    waitCycles++;
                    
                    if (waitCycles >= maxWaitCycles) {
                        std::cout << "\nTimeout reached. Some tasks may still be processing." << std::endl;
                        break;
                    }
                }
                
                std::cout << "\nTask processing completed (or timed out)." << std::endl;
            } else {
                std::cout << "No tasks were submitted." << std::endl;
            }
            
            std::cout << "Press Enter to exit and cleanup resources..." << std::endl;
            std::cin.get();
            
        } else {
            std::cout << "Invalid directory path: " << directory << std::endl;
            return 1;
        }
    } catch (const fs::filesystem_error& ex) {
        std::cout << "Filesystem error: " << ex.what() << std::endl;
        return 1;
    } catch (const std::exception& ex) {
        std::cout << "Error: " << ex.what() << std::endl;
        return 1;
    }
    
    std::cout << "Program completed successfully." << std::endl;
    return 0;
}