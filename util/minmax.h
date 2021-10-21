#pragma once


static inline int intmin(int x, int y) {
	return x < y ? x : y;
}

static inline int intmax(int x, int y) {
	return x > y ? x : y;
}

static inline int intmin3(int x, int y, int z) {
	if (x < y)
		if (x < z)
			return x;
		else
			return z;
	else if (y < z)
		return y;
	else
		return z;
}

static inline int intmax3(int x, int y, int z) {
	if (x > y)
		if (x > z)
			return x;
		else
			return z;
	else if (y > z)
		return y;
	else
		return z;
}
