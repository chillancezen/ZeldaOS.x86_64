/*
 * Copyright (c) 2019 Jie Zheng
 *
 *  Again this is not full functional paging for the kernel. only part of the
 *  core functions are implemented.
 *  Let's first map the 0-2G with 2M pages and part of 3G barrier backed memory
 *  into kernelspace in order to perform some special tasks, for example, LAPIC
 *  configuration. Map them in an exact way.
 */

#include <memory/include/paging.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>

uint64_t pml4_base;

void
paging_init(void)
{
    pml4_base = get_physical_page();
    memset((void *)pml4_base, 0x0, PAGE_SIZE_4K);
    LOG_INFO("New PML4 base:0x%x\n", pml4_base);
    LOG_INFO("pml4 size:%d\n", sizeof(struct pml4_entry));
}
