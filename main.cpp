#include <iostream>
#include "heap.h"

int main() {
	initHeap();
	int* ptr = (int*)allocate(sizeof(int));
	*ptr = 459;
	std::cout << *ptr << "\n";

	void* ptr2 = allocate(49);
	void* ptr3 = allocate(21);
	print_heap();
	deallocate(ptr3);
	deallocate(ptr);
	print_heap();
	deallocate(ptr2);
	print_heap();
	return 0;
}

