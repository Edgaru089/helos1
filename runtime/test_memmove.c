#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdlib.h>

__attribute__((ms_abi)) void *memmove(void *, const void *, size_t);
__attribute__((ms_abi)) void *memset(void *, int, size_t);

void output_memmove(const char *str) {
	puts(str);
}

int main(void) {
	char str[] = "1234567890";
	puts(str);
	memmove(str + 4, str + 3, 3); // copy from [4,5,6] to [5,6,7]
	puts(str);
	memmove(str + 3, str + 4, 3); // copy from [4,5,6] to [5,6,7]
	puts(str);
	memset(str + 3, '0' + 7, 3);
	puts(str);

	for (int i = 0; i <= 10; i++)
		printf("str[%d]=%d (%c)\n", i, str[i], str[i]);

	// setting effective type of allocated memory to be int
	int *p      = malloc(3 * sizeof(int)); // allocated memory has no effective type
	int  arr[3] = {1, 2, 3};
	memmove(p, arr, 3 * sizeof(int)); // allocated memory now has an effective type
	printf("%d %d %d\n", p[0], p[1], p[2]);

	// reinterpreting data
	double d = 0.1;
	//    int64_t n = *(int64_t*)(&d); // strict aliasing violation
	int64_t n;
	memmove(&n, &d, sizeof d); // OK
	printf("%a is %" PRIx64 " as an int64_t\n", d, n);
}
