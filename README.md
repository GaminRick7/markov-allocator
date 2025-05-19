# Markov-Guided Heap Allocator

This project implements a custom heap memory allocator in C++ that uses a Markov predictor to guess future allocation sizes and optimize memory reuse. It includes basic memory management operations like allocation, deallocation, coalescing of free blocks, and a visualization of the heap's state.

---

## Features

- **Custom Heap Initialization**: Uses `mmap` to create a fixed-size heap.
- **Alignment-Aware Allocation**: Ensures allocated memory blocks are aligned to 8 bytes.
- **Headers and Footers**: Each memory block includes metadata to track size and allocation status.
- **Free Block Coalescing**: Merges adjacent free blocks to reduce fragmentation.
- **Markov Predictor**: Learns patterns in allocation sizes to predict and cache likely future sizes.
- **Heap Visualization**: Prints a detailed view of heap state, showing size and allocation status of each block.

---

## File Structure

├── allocator.cpp # Core allocation and deallocation logic
├── heap.cpp # Heap initialization and memory operations
├── heap.h # Declarations for heap functions
├── MarkovPredictor.cpp # Markov predictor implementation
├── MarkovPredictor.h # Predictor class declaration
├── main.cpp # Sample usage and testing
└── README.md # Project documentation


---

## How It Works

### Initialization

- `initHeap()` sets up a 4KB memory region using `mmap`.
- The heap starts as one large free block, using headers and footers to track size and allocation status.

### Allocation (`allocate`)

- Aligns the requested size to 8 bytes.
- Uses `std::bit_ceil` to round up the size to the next power of two, grouping similar allocation requests together.
- Predicts whether a previously freed block of a certain size might be reused (based on the Markov model).
- If prediction is correct, reuses a cached block.
- If not, finds a suitable free block, possibly splitting it.
- Sets appropriate headers and footers to mark the block as allocated.

### Deallocation (`deallocate`)

- Marks the block as free by updating headers and footers.
- Before merging with neighbors (coalescing), checks the Markov predictor to see what the most likely next allocation size will be.
- If the just-freed block matches the predicted size class, it is temporarily *cached* instead of immediately merged with neighbors.
- This cache allows for faster future allocations of that size by skipping the usual free list traversal.
- If the block doesn't match the predicted next size, it is merged with adjacent free blocks to reduce fragmentation.

---

## Markov Predictor

The Markov predictor models the probability of requesting a certain size after having requested another.

- Each size class is represented as the log2 of the aligned size (e.g., 64 bytes → class 6).
- The predictor maintains a transition matrix `T[i][j]`, counting how often a request of class `j` follows class `i`.
- `predict(a)` returns the most frequent `b` observed after `a`.
- `update(a, b)` increments the count for transition `a → b`.

### Integration into Allocation & Deallocation

- **During Allocation**: If a predicted size matches a cached block, the allocator skips the free list search and reuses it directly.
- **During Deallocation**: If the just-freed block matches the predicted *next* size (based on the most recent allocation), it is stored in the cache instead of being merged into larger free blocks—preserving its utility.

This mechanism improves performance by reusing blocks before they are fragmented or coalesced into larger regions.

---
## Build Instructions

1. Clone the repository.
2. Compile with g++ (C++20 is required for `std::bit_ceil`):
   ```sh
   g++ -std=c++20 main.cpp allocator.cpp heap.cpp MarkovPredictor.cpp -o allocator
```
3.Run the executable:
```sh
./allocator
```


