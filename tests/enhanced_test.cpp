#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>
#include "heap.h"

void test_enhanced_markov_prediction() {
    std::cout << "\n=== Testing Enhanced Markov Prediction ===\n";
    
    // Train the predictor with a simple pattern: 8 -> 16
    std::vector<void*> pattern1;
    for (int i = 0; i < 3; i++) {
        void* p1 = allocate(8);
        void* p2 = allocate(16);
        pattern1.push_back(p1);
        pattern1.push_back(p2);
    }
    
    std::cout << "Trained pattern: 8 -> 16 (3 times)\n";
    
    // Deallocate to train the predictor
    for (size_t i = 0; i < pattern1.size(); i += 2) {
        deallocate(pattern1[i]);     // 8
        deallocate(pattern1[i + 1]); // 16
    }
    
    std::cout << "\nNow testing the learned pattern:\n";
    
    // Test the pattern - should see enhanced cache hits
    for (int i = 0; i < 2; i++) {
        std::cout << "\nPattern test " << (i + 1) << ":\n";
        void* p1 = allocate(8);
        std::cout << "  Allocated 8 bytes\n";
        deallocate(p1);
        
        void* p2 = allocate(16);
        std::cout << "  Allocated 16 bytes\n";
        deallocate(p2);
    }
}

void test_enhanced_caching() {
    std::cout << "\n=== Testing Enhanced Caching Strategy ===\n";
    
    // Allocate blocks of the same size
    std::vector<void*> blocks;
    for (int i = 0; i < 4; i++) {
        void* ptr = allocate(16);
        blocks.push_back(ptr);
    }
    
    std::cout << "Allocated 4 blocks of 16 bytes each\n";
    print_heap();
    
    // Deallocate every other block to create fragmentation
    for (int i = 0; i < 4; i += 2) {
        deallocate(blocks[i]);
    }
    
    std::cout << "\nAfter deallocating every other block:\n";
    print_heap();
    
    // Now allocate the same size - should see cache hits
    std::cout << "\nAllocating 16-byte blocks (should see cache hits):\n";
    for (int i = 0; i < 2; i++) {
        void* ptr = allocate(16);
        std::cout << "  Allocated 16 bytes\n";
        deallocate(ptr);
    }
    
    // Deallocate remaining blocks
    for (int i = 1; i < 4; i += 2) {
        deallocate(blocks[i]);
    }
    
    std::cout << "\nAfter deallocating all blocks:\n";
    print_heap();
}

int main() {
    std::cout << "=== Enhanced Markov Allocator Tests ===\n";
    std::cout << "Features: Eigen3-based prediction, enhanced caching, smart coalescing\n\n";
    
    initHeap();
    
    test_enhanced_markov_prediction();
    test_enhanced_caching();
    
    std::cout << "\n=== All enhanced tests completed ===\n";
    std::cout << "The allocator now features:\n";
    std::cout << "- Advanced Eigen3-based Markov prediction\n";
    std::cout << "- Enhanced caching with block splitting\n";
    std::cout << "- Adjacent block caching\n";
    std::cout << "- Smart coalescing that preserves cache opportunities\n";
    
    return 0;
}
