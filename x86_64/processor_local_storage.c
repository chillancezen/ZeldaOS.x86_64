/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <x86_64/include/processor_local_storage.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>
#include <x86_64/include/msr.h>
#include <x86_64/include/lapic.h>

static struct cpu_local_area pls_blob[MAX_NR_CPUS];

static void
processor_local_indicator_init(void)
{
    // Here we set gs.base as the offset, we should never modify it since
    // It's allowed to move a null selector to gs and the hidden part can be
    // modified by MSR mapping
    __asm__ volatile("xorq %%rax, %%rax;"
                     "mov %%rax, %%gs;"
                     :
                     :
                     :"%rax", "cc");
    uint32_t ecx, eax, edx;
    int this_cpu = cpu();
    ecx = GSBASE_MSR;
    eax = this_cpu * sizeof(struct cpu_local_area);
    edx = 0;
    WRMSR(ecx, eax, edx);
    ecx = KERNEL_GSBASE_MSR;
    WRMSR(ecx, eax, edx);
}

struct cpu_local_area *
get_cpu_local_area(void)
{
    uint16_t processor_index = 0;
    __asm__ volatile("movw %%gs:(%%rdx), %%ax;"
                     :"=a"(processor_index)
                     :"d"(pls_blob));
    ASSERT(processor_index >= 0 && processor_index < MAX_NR_CPUS);
    return &pls_blob[processor_index];
}

/*
 * This fast function to retrieve the cpu id, this function can only be called
 * after cpu local storage is initialized
 * XXX: as an enhancement, here we remove the restriction that the function is
 * only able to be called after the CPU local storage is initialized. 
 */
int
cpuid(void)
{
    struct cpu_local_area * _pls = get_cpu_local_area();
    if (UNLIKELY(_pls->magic_number != CPU_LOCAL_STORAGE_MAGIC)) {
        // It's much slower if the cpu local storage is not initialized.
        // but it's worthy, we need to get the cpu index at any stage..
        return cpu();   
    }
    return get_cpu_local_area()->processor_index;
}

void
processor_local_storage_init(void)
{
    processor_local_indicator_init();
    {
        int _cpu = cpu();
        pls_blob[_cpu].processor_index = _cpu;
        pls_blob[_cpu].magic_number = CPU_LOCAL_STORAGE_MAGIC;
    }
}

void
processor_local_storage_ap_init(void)
{
    processor_local_storage_init();
    {
        int _cpu = cpu();
        struct cpu_local_area * _pls = get_cpu_local_area();
        ASSERT(_pls->processor_index == _cpu);
        ASSERT(_pls == &pls_blob[_cpu]);
        LOG_INFO("cpu local stoage initialization finished for cpu:%d("
                 "storage base:0x%x)\n", cpuid(), _pls);
    }
}


__attribute__((constructor)) static void
processor_local_storage_pre_init(void)
{
    memset(pls_blob, 0x0, sizeof(pls_blob));
}

