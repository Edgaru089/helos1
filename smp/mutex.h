#pragma once

#include "../main.h"
#include "../util/vector.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


// Mutex is a mutual exclusion, a lock.
typedef struct {
	uintptr_t      locked;  // Zero for unlocked, non-zero for locked
	vector_Vector *waiting; // Threads waiting, __smp_Thread*
} smp_Mutex;

// Create a new Mutex.
smp_Mutex *smp_Mutex_Create();

// Destroy a Mutex, releasing its memory including itself.
void smp_Mutex_Destroy(smp_Mutex *mutex);


// Lock a mutex, and wait if it is already locked.
void smp_Mutex_Lock(smp_Mutex *mutex);

// LockTimeout Attempts to acquire a mutex in the given ticks.
// It returns true on lock acquire, or false on timeout.
//
// A timeout of 0 acts the same as Lock().
bool smp_Mutex_LockTimeout(smp_Mutex *mutex, uintptr_t timeout);

// TryLock attempts to acquire a mutex, and return false immediately if failed.
bool smp_Mutex_TryLock(smp_Mutex *mutex);

// Unlock a locked mutex.
void smp_Mutex_Unlock(smp_Mutex *mutex);


#ifdef __cplusplus
}
#endif
