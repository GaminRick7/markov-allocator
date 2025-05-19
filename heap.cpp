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
	size_t total_size = align(request_size) + 2 * HEADER_SIZE;
	char* curr = heapStart;

	if (std::bit_ceil(static_cast<unsigned int>(request_size)) == cache_guess) {
		curr = reinterpret_cast<char*>(cache_ptr);
		size_t header = *(reinterpret_cast<size_t*>(cache_ptr));
		size_t block_size = get_block_size(header);
		set_header(curr, block_size, true);
		set_header(curr + block_size - HEADER_SIZE, block_size, true);
		return curr + HEADER_SIZE;
	}

	if (prev_guess != -1) {
		predictor.update(log2(prev_guess), log2(std::bit_ceil(static_cast<unsigned int>(request_size))));
	}

	prev_guess = std::bit_ceil(static_cast<unsigned int>(request_size));

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

void coallesce(char* block, size_t size){
	if (std::bit_ceil(static_cast<unsigned int>(size - 2 * HEADER_SIZE)) == cache_guess) {
		std::cout << std::bit_ceil(static_cast<unsigned int>(size - 2 * HEADER_SIZE)) << std::endl;
		cache_ptr = block;
		return;
	}
	if (block != heapStart){
		char* prev_footer = block - HEADER_SIZE;
		size_t prev_size = get_block_size(*(reinterpret_cast<size_t*>(prev_footer)));
		if (!is_allocated(*(reinterpret_cast<size_t*>(prev_footer)))){
			block = prev_footer - prev_size + HEADER_SIZE;
			size += prev_size;
			set_header(block, size, false);
			set_header(block + size - HEADER_SIZE, size, false);
		}
	}

	if (block + size < heapStart + heapSize) {
		char* next_header = block + size;
		size_t next_size = get_block_size(*(reinterpret_cast<size_t*>(next_header)));
		if (!is_allocated(*(reinterpret_cast<size_t*>(next_header)))){
			size += next_size;
			set_header(block, size, false);
			set_header(block + size - HEADER_SIZE, size, false);
		}
	}
}

void deallocate(void* ptr){
	cache_guess = pow(2, predictor.predict(log2(prev_guess)));
	std::cout << cache_guess;
	char* block = reinterpret_cast<char*>(ptr) - HEADER_SIZE;
	size_t size = get_block_size(*(reinterpret_cast<size_t*>(block)));
	char* footer = block + size - HEADER_SIZE;
	set_header(block, size, false);
	set_header(footer, size, false);
	coallesce(block, size);
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

