
#include "unicode.h"


size_t utf8_Decode(const char *utf8, size_t len, uint32_t *codepoint) {
	// Some useful precomputed data
	static const int trailing[256] =
		{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
		 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
		 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5};
	static const uint32_t offsets[6] =
		{0x00000000, 0x00003080, 0x000E2080, 0x03C82080, 0xFA082080, 0x82082080};

	// decode the character
	uint32_t output;
	int      trailingBytes;

	// read the first byte
	uint8_t first = *utf8;
	trailingBytes = trailing[first];
	if ((first >= HELOS_UTF8_CONTINUATION_MIN && first <= HELOS_UTF8_OVERLONG_LEADER_2) ||
		(first >= HELOS_UTF8_INVALID_LEADER_MIN && first <= HELOS_UTF8_INVALID_LEADER_MAX) ||
		trailingBytes + 1 > len) {
		// corrupted data or incomplete character
		trailingBytes = 0;
		if (codepoint != 0)
			(*codepoint) = HELOS_UNICODE_ERROR;
	} else if (codepoint != 0) {
		output = 0;

		// so elegant!
		switch (trailingBytes) {
			case 5: output += (uint8_t)(*utf8++); output <<= 6;
			case 4: output += (uint8_t)(*utf8++); output <<= 6;
			case 3: output += (uint8_t)(*utf8++); output <<= 6;
			case 2: output += (uint8_t)(*utf8++); output <<= 6;
			case 1: output += (uint8_t)(*utf8++); output <<= 6;
			case 0: output += (uint8_t)(*utf8++);
		}

		(*codepoint) = output - offsets[trailingBytes];
	}

	return trailingBytes + 1;
}

size_t utf8_EncodeLength(uint32_t codepoint) {
	if (codepoint <= 0x007f) // 0000 ~ 007F
		return 1;
	else if (codepoint <= 0x07ff) // 0080 ~ 07FF
		return 2;
	else if (codepoint <= 0xffff) // 0800 ~ FFFF
		return 3;
	else if (codepoint <= 0x10ffff) // 10000 ~ 10FFFF
		return 4;

	return 0; // invalid
}

size_t utf8_Encode(char *utf8, uint32_t codepoint) {
	// Some useful precomputed data
	static const uint8_t firstBytes[7] = {0x00, 0x00, 0xC0, 0xE0, 0xF0, 0xF8, 0xFC};

	if (codepoint > HELOS_UNICODE_MAX)
		codepoint = HELOS_UNICODE_ERROR; // substitute invalid codepoint

	// get the number of bytes to write
	size_t len = utf8_EncodeLength(codepoint);
	// write the bytes
	// so elegant also!
	switch (len) {
		case 4: utf8[3] = (char)((codepoint | 0x80) & 0xBF); codepoint >>= 6;
		case 3: utf8[2] = (char)((codepoint | 0x80) & 0xBF); codepoint >>= 6;
		case 2: utf8[1] = (char)((codepoint | 0x80) & 0xBF); codepoint >>= 6;
		case 1: utf8[0] = (char)(codepoint | firstBytes[len]);
	}

	return len;
}
