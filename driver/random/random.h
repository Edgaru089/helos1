#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


void random_Seed(uint32_t seed);

uint32_t random_Rand();


#ifdef __cplusplus
}
#endif
