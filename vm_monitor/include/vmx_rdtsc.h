/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VMX_RDTSC_H
#define _VMX_RDTSC_H

#include <vm_monitor/include/vmx_exit.h>

uint64_t
rdtsc_exit_sub_handler(struct vmexit_info * exit);
#endif
