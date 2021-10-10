#include <stdio.h>
#include <stdint.h>

static inline uint64_t fillBits(int begin, int last) {
	if (last == 63)
		return ~((1ull << begin) - 1ull);
	if (begin == 0)
		return (1ull << (last + 1ull)) - 1ull;
	return (~((1ull << begin) - 1ull)) & ((1ull << (last + 1ull)) - 1ull);
}

void printit(int a, int b) {
	printf("[%d,%d] = %08lx\n", a, b, fillBits(a, b));
}

int main() {
	printit(0, 63);
	printit(1, 63);
	printit(2, 63);
	printit(3, 63);
	printit(4, 63);
	printit(5, 63);
	printit(6, 63);
	printit(7, 63);
	printit(8, 63);
	printit(9, 63);
	printit(10, 63);
	printit(7, 62);
	printit(8, 61);
	printit(9, 60);
	printit(10, 59);
	printit(10, 58);
	printit(10, 57);
	printit(10, 56);
	printit(10, 55);
	printit(10, 54);
	printit(4, 4);
}
