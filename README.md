## CPP_Memory_Leak      
A lightweight C++ memory tracking utility designed to help developers debug memory leaks. It tracks memory allocations and deallocations, detects unfreed memory (leaks), and provides detailed reports grouped by allocation type, file, and line number.

## Why CPP_Memory_Leak?           
C++ gives developers control over memory management, but this flexibility comes with the risk of memory leaks, double frees, and inefficient memory handling. MemoryTracker helps developers:

Detect memory leaks during development.
Debug memory-related issues by providing detailed reports.
Automatically clean up leaked memory to prevent undefined behavior.     

## Compilation
clang++ -std=c++14 -g main.cpp -o memorytracker
For testing modify main.cpp file. Create some memory leaks. Do compile as mentioned above. Run the exe file.
./exe

 ## Output
![Screenshot (2)](https://github.com/karma659/CppMemory_Leak/blob/main/uploads/1.png)
![Screenshot (1)](https://github.com/karma659/CppMemory_Leak/blob/main/uploads/3.png)
![Screenshot (3)](https://github.com/karma659/CppMemory_Leak/blob/main/uploads/2.png)
