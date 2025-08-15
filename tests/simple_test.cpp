#include <iostream>
#include "heap.h"

int main() {
    std::cout << "=== Simple Test for Enhanced Allocator ===\n";
    
    initHeap();
    std::cout << "Heap initialized successfully\n";
    
    // Simple allocation
    void* ptr1 = allocate(16);
    std::cout << "Allocated 16 bytes\n";
    
    void* ptr2 = allocate(32);
    std::cout << "Allocated 32 bytes\n";
    
    print_heap();
    
    // Simple deallocation
    deallocate(ptr1);
    std::cout << "Deallocated ptr1\n";
    
    deallocate(ptr2);
    std::cout << "Deallocated ptr2\n";
    
    print_heap();
    
    std::cout << "Test completed successfully\n";
    return 0;
}

