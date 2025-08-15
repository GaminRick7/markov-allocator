#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>
#include "heap.h"

int main() {
    std::cout << "=== Markov-Guided Heap Allocator Demo ===\n\n";
    
    // Initialize the heap
    initHeap();
    std::cout << "\n--- Phase 1: Initial allocations ---\n";
    
    // Allocate some memory blocks
    int* ptr1 = (int*)allocate(sizeof(int));
    *ptr1 = 100;
    std::cout << "Allocated int: " << *ptr1 << std::endl;
    
    char* ptr2 = (char*)allocate(64);
    std::strcpy(ptr2, "Hello, allocator!");
    std::cout << "Allocated string: " << ptr2 << std::endl;
    
    double* ptr3 = (double*)allocate(sizeof(double));
    *ptr3 = 3.14159;
    std::cout << "Allocated double: " << *ptr3 << std::endl;
    
    print_heap();
    
    std::cout << "\n--- Phase 2: Deallocate and observe predictions ---\n";
    
    // Deallocate in reverse order to see predictions
    deallocate(ptr3);
    print_heap();
    
    deallocate(ptr1);
    print_heap();
    
    std::cout << "\n--- Phase 3: Demonstrate pattern learning ---\n";
    
    // Create a repeating pattern to train the Markov predictor
    std::vector<void*> pattern_ptrs;
    
    for (int i = 0; i < 5; i++) {
        // Pattern: int -> double -> char array
        int* p1 = (int*)allocate(sizeof(int));
        *p1 = i;
        pattern_ptrs.push_back(p1);
        
        double* p2 = (double*)allocate(sizeof(double));
        *p2 = i * 1.5;
        pattern_ptrs.push_back(p2);
        
        char* p3 = (char*)allocate(32);
        std::snprintf(p3, 32, "Pattern %d", i);
        pattern_ptrs.push_back(p3);
    }
    
    std::cout << "Created allocation pattern (int -> double -> char[32]) 5 times\n";
    print_heap();
    
    std::cout << "\n--- Phase 4: Test prediction accuracy ---\n";
    
    // Deallocate pattern to train predictor
    for (size_t i = 0; i < pattern_ptrs.size(); i += 3) {
        deallocate(pattern_ptrs[i]);     // int
        deallocate(pattern_ptrs[i + 1]); // double
        deallocate(pattern_ptrs[i + 2]); // char array
    }
    
    print_heap();
    
    std::cout << "\n--- Phase 5: Test cache hits ---\n";
    
    // Now allocate the same pattern - should see cache hits
    for (int i = 0; i < 3; i++) {
        std::cout << "\nAllocation " << (i + 1) << ":\n";
        int* p1 = (int*)allocate(sizeof(int));
        *p1 = i * 10;
        std::cout << "Allocated int: " << *p1 << std::endl;
        
        double* p2 = (double*)allocate(sizeof(double));
        *p2 = i * 10.5;
        std::cout << "Allocated double: " << *p2 << std::endl;
        
        char* p3 = (char*)allocate(32);
        std::snprintf(p3, 32, "Test %d", i);
        std::cout << "Allocated string: " << p3 << std::endl;
        
        // Clean up immediately to continue pattern
        deallocate(p1);
        deallocate(p2);
        deallocate(p3);
    }
    
    print_heap();
    
    std::cout << "\n--- Final heap state ---\n";
    print_heap();
    
    std::cout << "\nDemo completed! The Markov predictor learned the allocation pattern\n";
    std::cout << "and should have cached blocks for faster reuse.\n";
    
    return 0;
}

