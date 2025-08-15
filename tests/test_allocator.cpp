#include <iostream>
#include <vector>
#include <cstring>
#include <cstdio>
#include "heap.h"

void test_pattern_learning() {
    std::cout << "\n=== Testing Pattern Learning ===\n";
    
    // Train the predictor with a specific pattern: 16 -> 32 -> 64
    std::vector<void*> pattern1;
    for (int i = 0; i < 3; i++) {
        void* p1 = allocate(16);
        void* p2 = allocate(32);
        void* p3 = allocate(64);
        pattern1.push_back(p1);
        pattern1.push_back(p2);
        pattern1.push_back(p3);
    }
    
    std::cout << "Trained pattern: 16 -> 32 -> 64 (3 times)\n";
    
    // Deallocate to train the predictor
    for (size_t i = 0; i < pattern1.size(); i += 3) {
        deallocate(pattern1[i]);     // 16
        deallocate(pattern1[i + 1]); // 32
        deallocate(pattern1[i + 2]); // 64
    }
    
    std::cout << "\nNow testing the learned pattern:\n";
    
    // Test the pattern - should see cache hits
    for (int i = 0; i < 2; i++) {
        std::cout << "\nPattern test " << (i + 1) << ":\n";
        void* p1 = allocate(16);
        std::cout << "  Allocated 16 bytes\n";
        deallocate(p1);
        
        void* p2 = allocate(32);
        std::cout << "  Allocated 32 bytes\n";
        deallocate(p2);
        
        void* p3 = allocate(64);
        std::cout << "  Allocated 64 bytes\n";
        deallocate(p3);
    }
}

void test_different_patterns() {
    std::cout << "\n=== Testing Different Patterns ===\n";
    
    // Train with a different pattern: 8 -> 128
    std::vector<void*> pattern2;
    for (int i = 0; i < 4; i++) {
        void* p1 = allocate(8);
        void* p2 = allocate(128);
        pattern2.push_back(p1);
        pattern2.push_back(p2);
    }
    
    std::cout << "Trained pattern: 8 -> 128 (4 times)\n";
    
    // Deallocate to train
    for (size_t i = 0; i < pattern2.size(); i += 2) {
        deallocate(pattern2[i]);     // 8
        deallocate(pattern2[i + 1]); // 128
    }
    
    std::cout << "\nTesting the new pattern:\n";
    
    // Test the new pattern
    for (int i = 0; i < 2; i++) {
        std::cout << "\nNew pattern test " << (i + 1) << ":\n";
        void* p1 = allocate(8);
        std::cout << "  Allocated 8 bytes\n";
        deallocate(p1);
        
        void* p2 = allocate(128);
        std::cout << "  Allocated 128 bytes\n";
        deallocate(p2);
    }
}

void test_fragmentation_reduction() {
    std::cout << "\n=== Testing Fragmentation Reduction ===\n";
    
    // Allocate many small blocks
    std::vector<void*> small_blocks;
    for (int i = 0; i < 10; i++) {
        void* ptr = allocate(16);
        small_blocks.push_back(ptr);
    }
    
    std::cout << "Allocated 10 blocks of 16 bytes each\n";
    print_heap();
    
    // Deallocate every other block to create fragmentation
    for (int i = 0; i < 10; i += 2) {
        deallocate(small_blocks[i]);
    }
    
    std::cout << "\nAfter deallocating every other block:\n";
    print_heap();
    
    // Deallocate remaining blocks
    for (int i = 1; i < 10; i += 2) {
        deallocate(small_blocks[i]);
    }
    
    std::cout << "\nAfter deallocating all blocks (coalescing should occur):\n";
    print_heap();
}

int main() {
    std::cout << "=== Advanced Markov Allocator Tests ===\n\n";
    
    initHeap();
    
    test_pattern_learning();
    test_different_patterns();
    test_fragmentation_reduction();
    
    std::cout << "\n=== All tests completed ===\n";
    return 0;
}

