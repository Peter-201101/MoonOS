#ifndef ARM_TIMER_H
#define ARM_TIMER_H

#include <types.h>

void     arm_timer_init(void);
uint64_t arm_timer_get_ticks(void);

#endif