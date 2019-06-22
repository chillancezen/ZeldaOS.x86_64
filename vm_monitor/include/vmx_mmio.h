/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VMX_MMIO_H
#define _VMX_MMIO_H
#include <vm_monitor/include/vmx_exit.h>

#define MAX_NR_MMIO_REGIONS 256


typedef uint64_t mmio_read_callback(uint64_t addr, int access_size,
                                    struct vmexit_info * exit);
typedef void mmio_write_callback(uint64_t addr, int access_size, uint64_t value,
                                 struct vmexit_info * exit);
struct mmio_operation {
    uint64_t addr_low;
    uint64_t addr_high;
    mmio_read_callback * mmio_read;
    mmio_write_callback * mmio_write;
};

#endif
