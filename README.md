# Markov-Guided Heap Allocator

This project implements a custom heap memory allocator in C++ that uses an advanced Eigen3-based Markov predictor to guess future allocation sizes and optimize memory reuse. It includes sophisticated memory management operations like allocation, deallocation, intelligent coalescing of free blocks, and comprehensive heap visualization.

---

## Features

- **Custom Heap Initialization**: Uses `mmap` to create a fixed-size heap.
- **Alignment-Aware Allocation**: Ensures allocated memory blocks are aligned to 8 bytes.
- **Headers and Footers**: Each memory block includes metadata to track size and allocation status.
- **Advanced Markov Prediction**: Uses Eigen3 matrix operations for sophisticated pattern learning.
- **Enhanced Caching Strategy**: Multiple caching strategies with block splitting and validation.
- **Smart Coalescing**: Cache-aware coalescing that preserves cache opportunities.
- **Heap Visualization**: Prints a detailed view of heap state, showing size and allocation status of each block.



## How the Complete System Works

### The Big Picture

The allocator works like a smart memory manager that learns from your allocation patterns and tries to predict what you'll need next. Think of it as having a very good memory - it remembers what sizes you typically request in sequence and prepares those blocks in advance.

### Initialization

When the allocator starts up, it asks the operating system for a 4KB chunk of memory using the `mmap` system call. This memory starts as one large free block with metadata at the beginning and end to track its size and status. The metadata includes both the block size and whether it's currently in use.

### The Allocation Process

When you request memory, the allocator follows a sophisticated decision-making process:

**First, it checks its cache.** If it has a block of the right size ready and waiting, it gives you that block immediately - this is the fastest possible allocation, taking constant time.

**If there's no cache hit, it cleans up any unused cached blocks** by merging them with neighboring free blocks to reduce fragmentation.

**Then it updates its prediction model.** The Markov predictor records the transition from your last allocation size to this one, building a pattern of how you typically use memory.

**Next comes the actual allocation.** It searches through the heap looking for a free block that's big enough. If it finds one that's much larger than needed, it splits it in half - giving you what you need and keeping the rest for future use.

**If no suitable block is found, it performs a comprehensive cleanup** by merging all adjacent free blocks, then tries the allocation again.

### The Deallocation Process

When you return memory, the allocator gets really smart about what to do with it:

**First, it marks the block as free** by updating the metadata.

**Then it makes a prediction.** Using the Markov model, it guesses what size you're most likely to request next based on your current allocation.

**Now it tries multiple caching strategies in order of preference:**

1. **Direct caching**: If the freed block exactly matches the predicted size, it caches it for immediate reuse.

2. **Adjacent block caching**: If the freed block doesn't match but a neighboring free block does, it caches that neighbor instead.

3. **Block splitting**: If the freed block is larger than predicted, it might split it to create a perfect match for caching.

**Only if none of these caching strategies work does it fall back to coalescing** - merging the freed block with its neighbors to create larger free blocks and reduce fragmentation.

### The Markov Prediction System

The heart of the allocator's intelligence is the Markov predictor. It works by observing patterns in your allocation behavior:

**Size Classification**: Instead of tracking exact byte sizes, it groups allocations into size classes based on powers of 2. For example, 4 bytes becomes class 2, 8 bytes becomes class 3, 16 bytes becomes class 4, and so on.

**Pattern Learning**: The predictor maintains two matrices - one counting how often each size class follows another, and another calculating the probability of these transitions. When you allocate memory, it updates these matrices to learn your patterns.

**Probability-Based Prediction**: When it needs to predict your next allocation, it uses matrix multiplication to calculate the probability of each possible next size class and picks the most likely one.

**Eigen3 Integration**: The sophisticated matrix operations are handled by the Eigen3 library, which provides optimized linear algebra functions for accurate probability calculations.

### The Enhanced Caching Strategy

The caching system is much more sophisticated than a simple "remember the last freed block" approach:

**Multiple Validation Strategies**: When considering whether to cache a block, it checks if the block is actually free, if it's the right size, and if it can be split to create the right size.

**Block Splitting for Cache**: If a freed block is larger than the predicted next allocation, it can split the block to create a perfect cache match, keeping the remainder for other uses.

**Adjacent Block Consideration**: It doesn't just look at the freed block - it also considers neighboring blocks that might be better candidates for caching.

**Cache Hit Scenarios**: Cache hits can occur in several ways: direct matches, split matches, adjacent matches, or even from coalesced blocks that happen to match the prediction.

### The Smart Coalescing Strategy

Coalescing (merging adjacent free blocks) is done intelligently to preserve cache opportunities:

**Cache-First Approach**: Before coalescing any blocks, it first tries to cache them if they match the predicted next allocation size.

**Coalescing with Cache Awareness**: When it does coalesce blocks, it tries to cache the resulting larger block if it matches the prediction.

**Comprehensive Cleanup**: The `coalesce_clean` function goes through the entire heap, coalescing all possible free blocks while still trying to preserve cache opportunities.

### A Complete Example

Let's trace through a typical scenario:

**Training Phase**: You allocate 16 bytes, then 32 bytes, then 64 bytes, then free them in the same order. The predictor learns that you often follow this pattern: 16 → 32 → 64 → 16.

**Execution Phase**: When you allocate 16 bytes again, the predictor guesses you'll want 32 bytes next, so it caches a 32-byte block. When you then allocate 32 bytes, it's a cache hit - instant allocation! The predictor then guesses you'll want 64 bytes next and caches accordingly.

**The Result**: Instead of searching through the heap each time, you get O(1) allocations when the predictions are correct, which happens frequently with repeated patterns.

### Performance Characteristics

**Time Complexity**: The best case is O(1) when there's a cache hit. The average case is O(log n) due to size class grouping, and the worst case is O(n) when it needs to traverse the entire heap and perform coalescing.

**Space Overhead**: Each block has 16 bytes of metadata (8-byte header and footer), plus alignment padding. The cache itself only requires a single block pointer and size class.

**Cache Hit Rates**: These improve dramatically with repeated patterns. The block splitting feature increases cache utilization, and adjacent caching captures spatial locality in your allocation patterns.

---

## Build Instructions

1. **Install Dependencies**:
   ```bash
   brew install eigen  # For Eigen3 matrix operations
   ```

2. **Build All Targets**:
   ```bash
   make all
   ```

3. **Run Tests**:
   ```bash
   make demo      # Run main demonstration
   make test      # Run basic tests
   make enhanced  # Run advanced feature tests
   ```

4. **Manual Compilation**:
   ```bash
   g++ -std=c++20 -I/opt/homebrew/include/eigen3 \
       main.cpp heap.cpp MarkovPredictor.cpp -o allocator
   ```




