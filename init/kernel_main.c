/*
 * Copyright (c) 2019 Jie Zheng
 *
 *    The init routines.
 */
#include <lib64/include/type.h>
#include <init/include/kernel.h>
#include <x86_64/include/gdt.h>
#include <lib64/include/logging.h>
#include <memory/include/physical_memory.h>
#include <memory/include/paging.h>
#include <x86_64/include/cpuid.h>
#include <x86_64/include/apic.h>

extern void * _kernel64_constructor_start;
extern void * _kernel64_constructor_end;

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
    physical_memory_init();
    paging_init();
}
static void
init1(void)
{
    check_basic_cpu_features();
    check_x2apic_mode();

    local_apic_init();
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
    LOG_INFO("Kernel is going to halt\n");
    printk("%s", kernel_banner);
    halt();
}


