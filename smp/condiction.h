#pragma once

#include "../main.h"
#include "../util/vector.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// Condition is a waiting condition.
typedef struct {
	vector_Vector *threads; // threads waiting on the condition
} smp_Condition;

// Create allocates a new, empty Condition.
smp_Condition *smp_Condition_Create();

void smp_Condition_Destroy(smp_Condition *con);

// Wait waits until the condition is notified.
//
// The returned data is (for now) useless.
void *smp_Condition_Wait(smp_Condition *con);

// NotifyOne unblocks (at most) one waiting thread.
//
// The data is (for now) not sent.
//
// If there are actually threads waiting, true is returned.
bool smp_Condition_NotifyOne(smp_Condition *con, void *data);

// NotifyAll unblocks all waiting threads.
//
// The data is (for now) not sent.
//
// Returns the number of threads unblocked.
int smp_Condition_NotifyAll(smp_Condition *con, void *data);

#ifdef __cplusplus
}
#endif
