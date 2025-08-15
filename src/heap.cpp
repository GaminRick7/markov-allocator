#include <iostream>
#include <sys/mman.h>
#include <bit>
#include <cmath>
#include <unistd.h>
#include "heap.h"
#include "MarkovPredictor.h"

constexpr size_t HEAP_SIZE = 4096;
constexpr size_t ALIGNMENT = 8;
constexpr size_t HEADER_SIZE = sizeof(size_t);

size_t heapSize = 0;
char* heapStart = nullptr;

int prev_guess = -1;
int cache_guess = -1;
void* cache_ptr = nullptr;

// Forward declarations
bool is_allocated(size_t header);
size_t get_block_size(size_t header);
void set_header(char* block, size_t size, bool allocated);

static bool validate_and_set_cache(void* ptr, int guess) {
    if (ptr == nullptr) return false;
    
    char* block = reinterpret_cast<char*>(ptr) - HEADER_SIZE;
    size_t block_size = get_block_size(*(reinterpret_cast<size_t*>(block)));
    bool allocated = is_allocated(*(reinterpret_cast<size_t*>(block)));
    
    if (allocated) return false;
    
    size_t user_size = block_size - 2 * HEADER_SIZE;
    int size_class = log2(std::bit_ceil(static_cast<unsigned int>(user_size)));
    
    if (size_class == guess) {
        cache_ptr = ptr;
        return true;
    } else if (size_class >= guess + 1) {
        // Split block for cache if it's larger than needed
        size_t target_size = (1 << guess) + 2 * HEADER_SIZE;
        if (block_size >= target_size + 2 * HEADER_SIZE + ALIGNMENT) {
            // Split the block
            set_header(block, target_size, false);
            set_header(block + target_size - HEADER_SIZE, target_size, false);
            
            size_t remainder = block_size - target_size;
            set_header(block + target_size, remainder, false);
            set_header(block + target_size + remainder - HEADER_SIZE, remainder, false);
            
            cache_ptr = block + HEADER_SIZE;
            return true;
        }
    }
    
    return false;
}

MarkovPredictor predictor;

bool is_allocated(size_t header) {
	return header & 1;
}

size_t get_block_size(size_t header) {
	return header & ~1;
}

void set_header(char* block, size_t size, bool allocated) {
	*(reinterpret_cast<size_t*> (block)) = size | (allocated ? 1: 0);
}

size_t align(size_t size) {
	return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

void initHeap(){
	if (heapStart != nullptr) return;

	heapStart = reinterpret_cast<char*>(mmap(nullptr, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0));
	if (heapStart == MAP_FAILED){
		std::cerr << "mmap failed to initialize heap";
		return;
	}
	heapSize = HEAP_SIZE;
	std::cout << "heap successfully initialized\n";

	*(reinterpret_cast<size_t*>(heapStart)) = HEAP_SIZE | 0;
	*(reinterpret_cast<size_t*>(heapStart + heapSize - HEADER_SIZE)) = HEAP_SIZE | 0;
}

void* allocate(size_t request_size) {
	if (request_size == 0) return nullptr;
	if (request_size > HEAP_SIZE - 2 * HEADER_SIZE) return nullptr;
	
	size_t total_size = align(request_size) + 2 * HEADER_SIZE;
	char* curr = heapStart;

	// Check cache first
	if (std::bit_ceil(static_cast<unsigned int>(request_size)) == static_cast<unsigned int>(cache_guess) && cache_ptr != nullptr) {
		std::cout << "CACHE HIT! Reusing cached block for size " << request_size << std::endl;
		curr = reinterpret_cast<char*>(cache_ptr) - HEADER_SIZE;
		size_t block_size = get_block_size(*(reinterpret_cast<size_t*>(curr)));
		set_header(curr, block_size, true);
		set_header(curr + block_size - HEADER_SIZE, block_size, true);
		cache_ptr = nullptr; // Clear cache after use
		cache_guess = -1;
		return curr + HEADER_SIZE;
	}

	// Coalesce any cached block that wasn't used
	if (cache_ptr != nullptr) {
		coalesce_one(reinterpret_cast<char*>(cache_ptr) - HEADER_SIZE);
	}
	cache_guess = 0;
	cache_ptr = nullptr;

	// Update Markov predictor
	if (prev_guess != -1) {
		predictor.update(log2(prev_guess), log2(std::bit_ceil(static_cast<unsigned int>(request_size))));
	}

	prev_guess = std::bit_ceil(static_cast<unsigned int>(request_size));

	// First-fit allocation
	while (curr < heapStart + HEAP_SIZE){
		size_t header = *(reinterpret_cast<size_t*> (curr));
		size_t block_size = get_block_size(header);
		bool allocated = is_allocated(header);

		if (!allocated && block_size >= total_size) {
			size_t remainder = block_size - total_size;
			if (remainder >= 2 * HEADER_SIZE + ALIGNMENT) {
				set_header(curr, total_size, true);
				set_header(curr + total_size - HEADER_SIZE, total_size, true);
				set_header(curr + total_size, remainder, false);
				set_header(curr + total_size + remainder - HEADER_SIZE, remainder, false);
				return curr + HEADER_SIZE;
			}
			set_header(curr, block_size, true);
			set_header(curr + block_size - HEADER_SIZE, block_size, true);
			return curr + HEADER_SIZE;
		}
		curr += block_size;
	}
	
	// If no block found, try coalescing and retry
	coalesce_clean();
	
	// Retry allocation after coalescing
	curr = heapStart;
	while (curr < heapStart + HEAP_SIZE){
		size_t header = *(reinterpret_cast<size_t*> (curr));
		size_t block_size = get_block_size(header);
		bool allocated = is_allocated(header);

		if (!allocated && block_size >= total_size) {
			size_t remainder = block_size - total_size;
			if (remainder >= 2 * HEADER_SIZE + ALIGNMENT) {
				set_header(curr, total_size, true);
				set_header(curr + total_size - HEADER_SIZE, total_size, true);
				set_header(curr + total_size, remainder, false);
				set_header(curr + total_size + remainder - HEADER_SIZE, remainder, false);
				return curr + HEADER_SIZE;
			}
			set_header(curr, block_size, true);
			set_header(curr + block_size - HEADER_SIZE, block_size, true);
			return curr + HEADER_SIZE;
		}
		curr += block_size;
	}
	
	return nullptr;
}

void coalesce_one(char* block) {
    if (!block) return;
    
    size_t block_size = get_block_size(*(reinterpret_cast<size_t*>(block)));
    
    // Try to cache the block first
    if (validate_and_set_cache(block + HEADER_SIZE, cache_guess)) {
        std::cout << "Caching block of size " << (1 << cache_guess) << " for predicted reuse" << std::endl;
        return;
    }
    
    // Coalesce with next block
    if (block + block_size < heapStart + heapSize) {
        char* next_header = block + block_size;
        size_t next_size = get_block_size(*(reinterpret_cast<size_t*>(next_header)));
        bool next_allocated = is_allocated(*(reinterpret_cast<size_t*>(next_header)));
        
        if (!next_allocated) {
            size_t new_size = block_size + next_size;
            set_header(block, new_size, false);
            set_header(block + new_size - HEADER_SIZE, new_size, false);
            
            // Try to cache the coalesced block
            if (validate_and_set_cache(block + HEADER_SIZE, cache_guess)) {
                std::cout << "Caching coalesced block of size " << (1 << cache_guess) << " for predicted reuse" << std::endl;
                return;
            }
        }
    }
    
    // Coalesce with previous block
    if (block != heapStart) {
        char* prev_footer = block - HEADER_SIZE;
        size_t prev_size = get_block_size(*(reinterpret_cast<size_t*>(prev_footer)));
        bool prev_allocated = is_allocated(*(reinterpret_cast<size_t*>(prev_footer)));
        
        if (!prev_allocated) {
            char* prev_block = prev_footer - prev_size + HEADER_SIZE;
            size_t new_size = prev_size + block_size;
            set_header(prev_block, new_size, false);
            set_header(prev_block + new_size - HEADER_SIZE, new_size, false);
            
            // Try to cache the coalesced block
            if (validate_and_set_cache(prev_block + HEADER_SIZE, cache_guess)) {
                std::cout << "Caching coalesced block of size " << (1 << cache_guess) << " for predicted reuse" << std::endl;
                return;
            }
        }
    }
}

void coalesce_clean() {
    char* curr = heapStart;
    
    while (curr < heapStart + heapSize) {
        size_t header = *(reinterpret_cast<size_t*>(curr));
        size_t block_size = get_block_size(header);
        bool allocated = is_allocated(header);
        
        if (!allocated) {
            coalesce_one(curr);
        }
        
        curr += block_size;
    }
}

void deallocate(void* ptr){
	if (ptr == nullptr) return;
	
	// Mark block as free
	char* block = reinterpret_cast<char*>(ptr) - HEADER_SIZE;
	size_t size = get_block_size(*(reinterpret_cast<size_t*>(block)));
	set_header(block, size, false);
	set_header(block + size - HEADER_SIZE, size, false);
	
	// Predict next allocation size
	cache_guess = pow(2, predictor.predict(log2(prev_guess)));
	std::cout << "Predicted next allocation size: " << cache_guess << std::endl;
	
	// Try to cache the freed block
	if (validate_and_set_cache(ptr, cache_guess)) {
		std::cout << "Caching freed block for predicted reuse" << std::endl;
		return;
	}
	
	// Try to cache adjacent blocks
	if (block + size < heapStart + heapSize) {
		char* next_header = block + size;
		if (validate_and_set_cache(next_header + HEADER_SIZE, cache_guess)) {
			std::cout << "Caching next block for predicted reuse" << std::endl;
			return;
		}
	}
	
	if (block != heapStart) {
		char* prev_footer = block - HEADER_SIZE;
		size_t prev_size = get_block_size(*(reinterpret_cast<size_t*>(prev_footer)));
		char* prev_block = prev_footer - prev_size + HEADER_SIZE;
		if (validate_and_set_cache(prev_block + HEADER_SIZE, cache_guess)) {
			std::cout << "Caching previous block for predicted reuse" << std::endl;
			return;
		}
	}
	
	// If no caching possible, coalesce
	coalesce_one(block);
	cache_guess = 0;
	cache_ptr = nullptr;
}

void print_heap() {
	std::cout << "Heap state:\n";
	char* curr = heapStart;
	size_t offset = 0;

	while (curr < heapStart + HEAP_SIZE) {
		size_t header = *((size_t*) curr);
		size_t block_size = get_block_size(header);
		bool allocated = is_allocated(header);

		std::cout << "Block at offset " << offset
		          << " | Size: " << block_size
		          << " | Allocated: " << (allocated ? "Yes" : "No") << "\n";

		if (block_size == 0 || block_size % ALIGNMENT != 0) {
			std::cerr << "Error: Invalid block size at offset " << offset << "\n";
			break;
		}
		curr += block_size;
		offset = curr - heapStart;
	}
	std::cout << std::endl;
}

