/*
 * Copyright (c) 2019 Jie Zheng
 * 
 *    To reference the cpu local storage data, we use GS.base as the offset to
 *    address the local storage area
 */

#ifndef _PROCESSOR_LOCAL_STORAGE
#define _PROCESSOR_LOCAL_STORAGE
#include <lib64/include/type.h>

// This structure are packed, do the fileds alignment ourselves inside the
// structure. and often the per-cpu local area is 4K page aligned.
struct cpu_local_area {
    // The first word must be cpu idndex.
    uint16_t processor_index;
    uint16_t tss_selector;
}__attribute__((packed, aligned(4096)));


int
cpuid(void);

void
processor_local_storage_init(void);

void
processor_local_storage_ap_init(void);
#endif
