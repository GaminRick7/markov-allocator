#pragma once

#include <cstddef>

void initHeap();
void* allocate(size_t size);
void deallocate(void* ptr);
void print_heap();


