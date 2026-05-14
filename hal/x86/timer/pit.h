#ifndef PIT_H
#define PIT_H

#include <types.h>

void     pit_init(void);
uint32_t pit_get_ticks(void);
void     pit_sleep(uint32_t ms);

#endif