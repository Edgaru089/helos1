#include <stdio.h>

#include "Unifont.h"

int main() {
	printf("Data at 0x%X\n\n", unifont_Data);

	for (int i = 0; i < 64; i++) {
		printf("%02d(%02X):", i, i);
		for (int j = 0; j < 32; j++) {
			printf("%02X", (unsigned int)unifont_Data[i * 32 + j]);
		}
		printf("\n");
	}


	return 0;
}
