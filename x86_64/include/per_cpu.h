/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _PER_CPU_H
#define _PER_CPU_H
#include <x86_64/include/processor_local_storage.h>

#define DECLARE_PER_CPU_VARIABLE(type, name) type name[MAX_NR_CPUS]

#define PER_CPU(_type, _name, _cpu) \
    ((_type *)((_cpu) * sizeof(_type) + (uint64_t)(_name)))


#define THIS_CPU(type, name) ({\
    int __this_cpu_id = cpuid(); \
    PER_CPU(type, name, __this_cpu_id); \
})


#endif
