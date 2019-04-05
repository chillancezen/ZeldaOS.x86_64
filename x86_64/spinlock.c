/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <x86_64/include/spinlock.h>
#include <lib64/include/string.h>
#include <x86_64/include/misc.h>
#include <x86_64/include/apic.h>
#include <lib64/include/logging.h>
void
spinlock_init(struct spinlock * lock)
{
    memset(lock, 0x0, sizeof(struct spinlock));
    lock->cpu = 0xff;
}



void
spinlock_acquire_raw(struct spinlock * lock)
{
    while (xchg(&lock->lock, 1) != 0);
    barrier();
    lock->cpu = cpu();
#if defined (DEBUG)
#endif
}

void
spinlock_release_raw(struct spinlock * lock)
{
    ASSERT(lock->cpu == cpu() && lock->lock == 1);
    lock->cpu = 0xff;
    barrier();
    ASSERT(xchg(&lock->lock, 0) == 1);
}

