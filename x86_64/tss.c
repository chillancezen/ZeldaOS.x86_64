/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <x86_64/include/apic.h>
#include <x86_64/include/tss.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>

__attribute__((aligned(64)))
struct task_state_segment64 tss_base[MAX_NR_CPUS];

struct task_state_segment64 *
get_tss_base(void)
{
    return tss_base;
}

void
set_pl0_rsp(int cpu, uint64_t rsp)
{
    ASSERT(cpu >=0 && cpu < MAX_NR_CPUS);
    tss_base[cpu].rsp0 = rsp;
}

void
load_pl0_task(int cpu)
{
    ASSERT(cpu >=0 && cpu < MAX_NR_CPUS);
    int selector_index = 5 + cpu;
    int selector = selector_index << 3;
    __asm__ volatile("ltr %%ax;"
                     :
                     :"a"(selector)
                     :"cc");
    {
        int _selector = 0;
        __asm__ volatile("str %%ax;"
                         :"=a"(_selector)
                         :
                         :"memory");
        ASSERT(_selector == selector);
    }
    LOG_INFO("load task register with selector:0x%x for cpu:%d\n",
             selector, cpu);
}

void
task_register_init(void)
{
    load_pl0_task(cpu());
}

__attribute__((constructor))
static void tss_pre_init(void)
{
    int idx = 0;
    memset(tss_base, 0x0, sizeof(tss_base));
    for (idx = 0; idx < MAX_NR_CPUS; idx++) {
        tss_base[idx].iomap_base = 0x0068;
    }
}




