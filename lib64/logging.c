/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <lib64/include/logging.h>

int __log_level = DEFAULT_LOGGING_LEVEL;

struct spinlock __logging_lock;


__attribute__((constructor)) void
logging_pre_init(void)
{
    spinlock_init(&__logging_lock);
}


