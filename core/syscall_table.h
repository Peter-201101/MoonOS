#ifndef SYSCALL_TABLE_H
#define SYSCALL_TABLE_H

#include <core/syscall.h>

void         syscall_table_init(void);
syscall_fn_t syscall_table_get(uint32_t num);

#endif