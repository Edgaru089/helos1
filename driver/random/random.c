
#include "random.h"

// glibc defaults
static const uint32_t a = 1103515245, c = 12345, m = 2147483648;
static uint32_t       v = 1;


void random_Seed(uint32_t seed) {
	v = seed;
}

uint32_t random_Rand() {
	return v = (uint32_t)(((uint64_t)v * a + c) % m);
}
