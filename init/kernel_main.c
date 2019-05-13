/*
 * Copyright (c) 2019 Jie Zheng
 *
 *    The init routines.
 */
#include <lib64/include/type.h>
#include <init/include/kernel.h>
#include <init/include/lapic_timer.h>
#include <x86_64/include/gdt.h>
#include <lib64/include/logging.h>
#include <memory/include/physical_memory.h>
#include <memory/include/paging.h>
#include <x86_64/include/cpuid.h>
#include <x86_64/include/lapic.h>
#include <x86_64/include/interrupt.h>
#include <x86_64/include/tss.h>
#include <x86_64/include/ioapic.h>
#include <device/include/keyboard.h>
#include <vm_monitor/include/vmx_misc.h>
#include <vm_monitor/include/vmx_vmcs.h>
#include <x86_64/include/processor_local_storage.h>
#include <x86_64/include/per_cpu.h>
#include <x86_64/include/panic.h>

extern void * _kernel64_constructor_start;
extern void * _kernel64_constructor_end;

DECLARE_PER_CPU_VARIABLE(uint32_t, foo);

static void
pre_init(void)
{
    uint64_t start_addr = (uint64_t)&_kernel64_constructor_start;
    uint64_t end_addr = (uint64_t)&_kernel64_constructor_end;
    uint64_t func_container = 0;
    for (func_container = start_addr;
         func_container < end_addr;
         func_container += 8) {
        ((void (*)(void))*(uint64_t *)func_container)();
    }
}


static void
init0(void)
{
    gdt64_init();
    task_register_init();
    physical_memory_init();
    paging_init();
}
static void
init1(void)
{
    check_basic_cpu_features();
    check_x2apic_mode();
    local_apic_init();
    io_apic_init();
    interrupt_init();
    start_other_processors();
    processor_local_storage_init();
}

static void
init2(void)
{
    lapic_timer_init();
    keyboard_init();
}

static void
init3(void)
{
    vm_monitor_init();
    {
        struct vmcs_blob vm;
        pre_initialize_vmcs(&vm);
        load_vmcs(&vm);
    }
}
static char * kernel_banner =
"▒███████▒▓█████  ██▓    ▓█████▄  ▄▄▄          ▒█████    ██████ \n"
"▒ ▒ ▒ ▄▀░▓█   ▀ ▓██▒    ▒██▀ ██▌▒████▄       ▒██▒  ██▒▒██    ▒ \n"
"░ ▒ ▄▀▒░ ▒███   ▒██░    ░██   █▌▒██  ▀█▄     ▒██░  ██▒░ ▓██▄   \n"
"  ▄▀▒   ░▒▓█  ▄ ▒██░    ░▓█▄   ▌░██▄▄▄▄██    ▒██   ██░  ▒   ██▒\n"
"▒███████▒░▒████▒░██████▒░▒████▓  ▓█   ▓██▒   ░ ████▓▒░▒██████▒▒\n"
"░▒▒ ▓░▒░▒░░ ▒░ ░░ ▒░▓  ░ ▒▒▓  ▒  ▒▒   ▓▒█░   ░ ▒░▒░▒░ ▒ ▒▓▒ ▒ ░\n"
"░░▒ ▒ ░ ▒ ░ ░  ░░ ░ ▒  ░ ░ ▒  ▒   ▒   ▒▒ ░     ░ ▒ ▒░ ░ ░▒  ░ ░\n"
"░ ░ ░ ░ ░   ░     ░ ░    ░ ░  ░   ░   ▒      ░ ░ ░ ▒  ░  ░  ░  \n"
"  ░ ░       ░  ░    ░  ░   ░          ░  ░       ░ ░        ░  \n"
"░                        ░                                     \n";

void
kernel_main(void)
{
    pre_init();
    init0();
    init1();
    init2();
    init3();
    LOG_INFO("Kernel is going to halt\n");
    printk("%s", kernel_banner);
    {
        set_pl0_rsp(0, get_physical_pages(1024));
    }
    sti();
    // Loop Forever in halted state
    while (1) {
        halt();
    }
}

void
kernel_ap_main(void)
{
    gdt64_ap_init();
    ap_paging_init();
    local_apic_ap_init();
    interrupt_ap_init();
    processor_local_storage_ap_init();

#if defined(DEBUG)
    {
        //This is to test and verify per-cpu utilization
        ASSERT(THIS_CPU(uint32_t, foo) == PER_CPU(uint32_t, foo, cpuid()))
    }
#endif
    sti();
    while (1) {
        halt();
    }
}

