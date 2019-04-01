/*
 * Copyright (c) 2019 Jie Zheng
 *
 * the function body to implement CPUID helper
 */

#include <x86_64/include/cpuid.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>


void
check_basic_cpu_features(void)
{
    uint32_t eax, ebx, ecx, edx;
    {
        // Examine the CPU vendor
        char vendor_string[32];
        eax = 0;
        memset(vendor_string, 0x0, sizeof(vendor_string));
        CPUID(eax, ebx, ecx, edx);
        *(uint32_t *)(vendor_string + 0) = ebx;
        *(uint32_t *)(vendor_string + 4) = edx;
        *(uint32_t *)(vendor_string + 8) = ecx;
        LOG_DEBUG("cpu vendor: %s\n", vendor_string);
    }

    {
        // Examine the MSR featutre
        eax = 1;
        CPUID(eax, ebx, ecx, edx);
        if (edx & CPUID_FEAT_EDX_MSR) {
            LOG_DEBUG("cpu feature:msr detected\n");
        } else {
            __not_reach();
        }
    }

    {
        // Examine the APIC feature
        eax = 1;
        CPUID(eax, ebx, ecx, edx);
        if (edx & CPUID_FEAT_EDX_APIC) {
            LOG_DEBUG("cpu feature:apic detected\n");
        } else {
            __not_reach();
        }
    }
}
