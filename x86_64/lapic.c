/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <x86_64/include/cpuid.h>
#include <x86_64/include/apic.h>
#include <lib64/include/logging.h>

static int is_x2apic_supported = 0;
void
check_x2apic_mode(void)
{
    uint32_t eax = 0x1, ebx = 0, ecx = 0, edx = 0;
    CPUID(eax, ebx, ecx, edx);
    if (ecx & CPUID_FEAT_ECX_x2APIC) {
        is_x2apic_supported = 1;
        LOG_DEBUG("cpu feature:x2apic detected\n");
    } else {
        is_x2apic_supported = 0;
        LOG_DEBUG("cpu feature:x2apic not supported\n");
    }
}




void
local_apic_init(void)
{
    LOG_INFO("local apic works in legacy mode\n");
}
