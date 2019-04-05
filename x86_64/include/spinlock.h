/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _SPINLOCK_H
#define _SPINLOCK_H
#include <x86_64/include/atomic.h>

struct spinlock {
    uint32_t lock;
    uint32_t cpu;
    void * blob;

#if defined(DEBUG)
#define MAX_CALLING_FRAME 32
    uint64_t backtrace[32];
#endif

};

void
spinlock_init(struct spinlock * lock);

void
spinlock_acquire_raw(struct spinlock * lock);

void
spinlock_release_raw(struct spinlock * lock);

#endif

