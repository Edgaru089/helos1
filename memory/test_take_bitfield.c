
#include <stdio.h>
#include <stdint.h>

inline static uint64_t takeBitfield(uint64_t v, int high, int low) {
	return (v >> low) & ((1 << (high - low + 1)) - 1);
}

int main() {
	uint64_t val = 0x400CD696E37AE;
	printf("val        = %lX\n", val);
	printf("val[21:5]  = %lX\n", takeBitfield(val, 21, 5));
	printf("val[33:12] = %lX\n", takeBitfield(val, 33, 12));
	printf("val[21:5] = %lX\n", takeBitfield(val, 21, 5));
	printf("val[21:5] = %lX\n", takeBitfield(val, 21, 5));
}
