/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VMX_MISC_H
#define _VMX_MISC_H
#include <lib64/include/type.h>

int
is_vmx_support(void);


void
vm_monitor_init(void);


uint32_t
get_vmx_revision_id(void);

#endif
