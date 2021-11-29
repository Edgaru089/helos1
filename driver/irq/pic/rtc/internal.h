#pragma once

#include "rtc.h"
#include "../../../../main.h"


extern void  *__pic_rtc_IRQHandler;
SYSV_ABI void __pic_rtc_IRQ8();
