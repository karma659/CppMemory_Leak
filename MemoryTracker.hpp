#ifndef MEMORY_TRACKER_HPP
#define MEMORY_TRACKER_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdlib>


class MemoryTracker {
public:

    struct Allocation {
        size_t size;         
        const char* file;    
        int line;             
        std::string type;     
        bool isFreed;         
    };

    static void* allocate(size_t size, const char* file, int line, const std::string& type) {
        void* ptr = std::malloc(size);  
        if (!ptr) {
            throw std::bad_alloc();     
        }

        Allocation alloc = {size, file, line, type, false}; 
        allocations[ptr] = alloc;                         
        totalMemoryAllocated += size;                   
        return ptr;
    }

    static void deallocate(void* ptr) {
        if (allocations.find(ptr) != allocations.end()) {
            Allocation& alloc = allocations[ptr];
            if (!alloc.isFreed) { 
                alloc.isFreed = true;
                totalMemoryFreed += alloc.size; 
                std::free(ptr);    
                allocations.erase(ptr); 
            } else {
                std::cerr << "Warning: Attempted to free already freed memory at pointer: " 
                          << ptr << std::endl;
            }
        } else {
            std::cerr << "Warning: Attempted to free untracked memory at pointer: " 
                      << ptr << std::endl;
        }
    }

    static void reportGroupedLeaks() {
        if (allocations.empty()) {
            std::cout << "No memory leaks detected." << std::endl;
            return;
        }

        std::unordered_map<std::string, std::pair<size_t, size_t>> groupedLeaks; 

        for (const auto& entry : allocations) {
            const Allocation& alloc = entry.second;
            if (!alloc.isFreed) {
                if (groupedLeaks.find(alloc.type) == groupedLeaks.end()) {
                    groupedLeaks[alloc.type] = {0, 0}; 
                }
                groupedLeaks[alloc.type].first += 1;       
                groupedLeaks[alloc.type].second += alloc.size; 
            }
        }

        std::cout << "\nMemory Leaks Detected (Grouped by Type):" << std::endl;
        for (const auto& group : groupedLeaks) {
            const std::string& type = group.first;
            size_t totalLeaks = group.second.first;
            size_t totalSize = group.second.second;

            std::cout << "Type: " << type 
                      << " | Leaks: " << totalLeaks 
                      << " | Total Size: " << totalSize << " bytes" 
                      << std::endl;
        }
    }

    static void freeLeakedMemory() {
        if (allocations.empty()) {
            std::cout << "\nNo memory to free. All memory has already been deallocated." << std::endl;
            return;
        }
        std::unordered_map<std::string, size_t> freedByType; 

        std::cout << "\nFreeing all leaked memory..." << std::endl;
        for (auto it = allocations.begin(); it != allocations.end();) {
            void* ptr = it->first;
            Allocation& alloc = it->second;

            if (!alloc.isFreed) { 
                std::cout << "Freeing memory of type: " << alloc.type
                          << ", Size: " << alloc.size
                          << ", File: " << alloc.file
                          << ", Line: " << alloc.line << std::endl;

                freedByType[alloc.type] += alloc.size;
                std::free(ptr);
                totalMemoryFreed += alloc.size;
                it = allocations.erase(it); 
            } else {
                ++it;
            }
        }

        std::cout << "\nSummary of Freed Memory:" << std::endl;
        for (const auto& entry : freedByType) {
            std::cout << "  Type: " << entry.first
                      << " | Total Freed Size: " << entry.second << " bytes" << std::endl;
        }
    }

    static void printMemoryUsage() {
        std::cout << "\nMemory Usage Report:" << std::endl;
        std::cout << "  Total Memory Allocated: " << totalMemoryAllocated << " bytes" << std::endl;
        std::cout << "  Total Memory Freed: " << totalMemoryFreed << " bytes" << std::endl;
        std::cout << "  Current Memory in Use: " 
                  << (totalMemoryAllocated - totalMemoryFreed) << " bytes" << std::endl;
    }

private:
    static std::unordered_map<void*, Allocation> allocations;
    static size_t totalMemoryAllocated;
    static size_t totalMemoryFreed;
};


std::unordered_map<void*, MemoryTracker::Allocation> MemoryTracker::allocations;
size_t MemoryTracker::totalMemoryAllocated = 0;
size_t MemoryTracker::totalMemoryFreed = 0;


#define NEW(size, type) MemoryTracker::allocate(size, __FILE__, __LINE__, type)
#define DELETE(ptr) MemoryTracker::deallocate(ptr)

#endif 
