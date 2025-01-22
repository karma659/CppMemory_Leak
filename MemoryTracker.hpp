#ifndef MEMORY_TRACKER_HPP
#define MEMORY_TRACKER_HPP

#include <iostream>
#include <unordered_map>
#include <string>
#include <cstdlib>
#include <cstring>
#define NEW(size, type) MemoryTracker::allocate(size, __FILE__, __LINE__, type)
#define DELETE(ptr) MemoryTracker::deallocate(ptr)

// MemoryTracker class
class MemoryTracker {
public:
    // Struct to hold memory allocation details
    struct Allocation {
        size_t size;          // Size of the allocated memory
        const char* file;     // Source file where the allocation occurred
        int line;             // Line number in the source file
        std::string type;     // Type of the allocation (e.g., "Malloc", "Calloc")
        bool isFreed;         // Flag to indicate if the memory was freed
    };

    // Allocates memory, tracks the allocation, and returns a pointer
    static void* allocate(size_t size, const char* file, int line, const std::string& type) {
        void* ptr = std::malloc(size);  // Allocate memory
        if (!ptr) {
            throw std::bad_alloc();     // Handle allocation failure
        }

        Allocation alloc = {size, file, line, type, false}; // Track allocation details
        allocations[ptr] = alloc;                          // Add to allocations map
        totalMemoryAllocated += size;                     // Update memory allocated
        return ptr;
    }

    // Deallocates memory and removes it from tracking
    static void deallocate(void* ptr) {
        if (allocations.find(ptr) != allocations.end()) { // Check if pointer is tracked
            Allocation& alloc = allocations[ptr];
            if (!alloc.isFreed) { // Mark as freed
                alloc.isFreed = true;
                totalMemoryFreed += alloc.size; // Update memory freed
                std::free(ptr);      // Free memory
                allocations.erase(ptr); // Remove from allocations map
            } else {
                std::cerr << "Warning: Attempted to free already freed memory at pointer: " 
                          << ptr << std::endl;
            }
        } else {
            std::cerr << "Warning: Attempted to free untracked memory at pointer: " 
                      << ptr << std::endl;
        }
    }

    // Reports memory leaks grouped by type and shows the total size of leaks for each type
    static void reportGroupedLeaks() {
        if (allocations.empty()) {
            std::cout << "No memory leaks detected." << std::endl;
            return;
        }

        std::unordered_map<std::string, std::pair<size_t, size_t>> groupedLeaks; 
        // Key: Type, Value: Pair (total leaks count, total size)

        for (const auto& entry : allocations) {
            const Allocation& alloc = entry.second;
            if (!alloc.isFreed) {
                if (groupedLeaks.find(alloc.type) == groupedLeaks.end()) {
                    groupedLeaks[alloc.type] = {0, 0}; // Initialize count and size
                }
                groupedLeaks[alloc.type].first += 1;        // Increment count
                groupedLeaks[alloc.type].second += alloc.size; // Add to total size
            }
        }

        // Print grouped leak report
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

    // Frees all remaining leaked memory and reports the total size for each type
    static void freeLeakedMemory() {
        if (allocations.empty()) {
            std::cout << "\nNo memory to free. All memory has already been deallocated." << std::endl;
            return;
        }

        // Group memory to be freed by type
        std::unordered_map<std::string, size_t> freedByType; // Key: Type, Value: Total size freed

        std::cout << "\nFreeing all leaked memory..." << std::endl;
        for (auto it = allocations.begin(); it != allocations.end();) {
            void* ptr = it->first;
            Allocation& alloc = it->second;

            if (!alloc.isFreed) { // Only free unfreed memory
                std::cout << "Freeing memory of type: " << alloc.type
                          << ", Size: " << alloc.size
                          << ", File: " << alloc.file
                          << ", Line: " << alloc.line << std::endl;

                freedByType[alloc.type] += alloc.size;
                std::free(ptr);
                totalMemoryFreed += alloc.size;
                it = allocations.erase(it); // Remove the allocation entry
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

    // Prints total memory usage statistics
    static void printMemoryUsage() {
        std::cout << "\nMemory Usage Report:" << std::endl;
        std::cout << "  Total Memory Allocated: " << totalMemoryAllocated << " bytes" << std::endl;
        std::cout << "  Total Memory Freed: " << totalMemoryFreed << " bytes" << std::endl;
        std::cout << "  Current Memory in Use: " 
                  << (totalMemoryAllocated - totalMemoryFreed) << " bytes" << std::endl;
    }

    // Helper function to retrieve the size of an allocated memory block
    static size_t getAllocationSize(void* ptr) {
        auto it = allocations.find(ptr);
        if (it != allocations.end()) {
            return it->second.size;
        }
        return 0; // Return 0 if the pointer is not tracked
    }

private:
    static std::unordered_map<void*, Allocation> allocations; // Tracks all allocations
    static size_t totalMemoryAllocated; // Tracks total allocated memory
    static size_t totalMemoryFreed; // Tracks total freed memory
};

// Initialize static members
std::unordered_map<void*, MemoryTracker::Allocation> MemoryTracker::allocations;
size_t MemoryTracker::totalMemoryAllocated = 0;
size_t MemoryTracker::totalMemoryFreed = 0;

// Wrapper functions for malloc, calloc, realloc, and free
inline void* trackedMalloc(size_t size) {
    return MemoryTracker::allocate(size, __FILE__, __LINE__, "Malloc");
}

inline void* trackedCalloc(size_t count, size_t size) {
    void* ptr = MemoryTracker::allocate(count * size, __FILE__, __LINE__, "Calloc");
    if (ptr) {
        std::memset(ptr, 0, count * size);
    }
    return ptr;
}

inline void* trackedRealloc(void* ptr, size_t newSize) {
    if (!ptr) {
        return MemoryTracker::allocate(newSize, __FILE__, __LINE__, "Realloc");
    }

    if (newSize == 0) {
        MemoryTracker::deallocate(ptr);
        return nullptr;
    }

    void* newPtr = MemoryTracker::allocate(newSize, __FILE__, __LINE__, "Realloc");
    if (newPtr) {
        size_t oldSize = MemoryTracker::getAllocationSize(ptr); // Use helper method
        std::memcpy(newPtr, ptr, std::min(oldSize, newSize));
        MemoryTracker::deallocate(ptr);
    }

    return newPtr;
}

inline void trackedFree(void* ptr) {
    MemoryTracker::deallocate(ptr);
}

// Macros to replace dynamic memory calls
#define malloc(size) trackedMalloc(size)
#define calloc(count, size) trackedCalloc(count, size)
#define realloc(ptr, size) trackedRealloc(ptr, size)
#define free(ptr) trackedFree(ptr)

#endif // MEMORY_TRACKER_HPP
