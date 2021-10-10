#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "Unicode.h"


int main(int argc, char *argv[]) {
	char str[] = {"天王盖地虎，宝塔镇河妖"};
	int  len   = strlen(str);

	printf("String: %s, Length: %d\n", str, len);

	uint32_t codepoint, buffer[128];
	int      count = 0;
	for (int i = 0;
		 i < len;
		 i += utf8_Decode(str + i, len - i, buffer + count), count++) {}


	printf("Count=%d\n", count);
	for (int i = 0; i < count; i++) {
		printf("Buffer[%d] = %d, ", i, buffer[i]);
		char bytes[5] = {};
		utf8_Encode(bytes, buffer[i]);
		printf("%s\n", bytes);
	}

	return 0;
}
