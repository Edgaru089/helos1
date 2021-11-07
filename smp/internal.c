
#include "internal.h"


uint64_t __smp_Now         = 1;
int      __smp_Count       = 1;
bool     __smp_PauseTicker = false;

__smp_Thread **__smp_Current;

tree_Tree *__smp_Threads;
tree_Tree *__smp_ThreadsWaiting;
