/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <x86_64/include/cpuid.h>
#include <x86_64/include/apic.h>
#include <lib64/include/logging.h>
#include <x86_64/include/msr.h>
#include <x86_64/include/ioport.h>

static int is_x2apic_supported = 0;


static inline uint32_t
lapic_read(uint32_t reg_address)
{
    uint32_t volatile * _reg_address =
        (uint32_t volatile *)(uint64_t)reg_address;
    return _reg_address[0];
}


static inline void
lapic_write(uint32_t reg_address, uint32_t value)
{
    uint32_t volatile * _reg_address =
        (uint32_t volatile *)(uint64_t)reg_address;
    _reg_address[0] = value;
    lapic_read(APIC_ID_REGISTER);  
}

inline uint32_t
cpu(void)
{
    uint32_t eax = 0x1, ebx, ecx, edx;
    CPUID(eax, ebx, ecx, edx);
    return ebx >> 24;
    // XXX: deprecate the code below as the APIC base space may not be mapped
    //uint32_t id_reg = lapic_read(APIC_ID_REGISTER);
    //return id_reg >> 24;
}

static inline void
mask_lvt_entry(uint32_t reg_address)
{
    uint32_t reg = lapic_read(reg_address);
    reg |= APIC_LVT_MASKED;
    lapic_write(reg_address, reg);
}

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

int
is_bootstrap_processor(void)
{
    uint32_t eax, edx;
    RDMSR(APIC_BASE_MSR, &eax, &edx);
    return !!(eax & APIC_MSR_BSP_FLAG);
}

uint32_t
local_apic_base(void)
{
    uint32_t eax, edx;
    RDMSR(APIC_BASE_MSR, &eax, &edx);
    return eax & 0xfffff000;
}

int
is_local_apic_enabled(void)
{
    uint32_t supurious_interrupt_register;
    uint32_t eax, edx;
    RDMSR(APIC_BASE_MSR, &eax, &edx);
    supurious_interrupt_register = lapic_read(APIC_SPURIOUS_REGISTER);
    return !!(eax & APIC_MSR_ENABLE_FLAG) &&
           supurious_interrupt_register & APIC_SPURIOUS_REGISTER_APIC_ENABLE;
}

/*
 * this function is to enable Local APIC by:
 *   1). enable global APIC enable flag in APIC_BASE msr
 *   2). enable the local apic flag in supurious interrupt vector register
 */
void
enable_local_apic(void)
{
    uint32_t svr;
    uint32_t eax, edx;
    RDMSR(APIC_BASE_MSR, &eax, &edx);
    if (!(eax & APIC_MSR_ENABLE_FLAG)) {
        eax |= APIC_MSR_ENABLE_FLAG;
        WRMSR(APIC_BASE_MSR, eax, edx);
        RDMSR(APIC_BASE_MSR, &eax, &edx);
    }
    ASSERT(eax & APIC_MSR_ENABLE_FLAG);

    svr = lapic_read(APIC_SPURIOUS_REGISTER);
    if (!(svr & APIC_MSR_ENABLE_FLAG)) {
        svr |= APIC_SPURIOUS_REGISTER_APIC_ENABLE;
        lapic_write(APIC_SPURIOUS_REGISTER, svr);
        svr = lapic_read(APIC_SPURIOUS_REGISTER);
    }
    ASSERT(svr & APIC_SPURIOUS_REGISTER_APIC_ENABLE);
}


/*
 */
static void
loop_and_wait(uint64_t times_to_loop)
{
    __asm__ volatile("1:nop;"
                     "loop 1b;"
                     :
                     :"c"(times_to_loop));
}

void
acknowledge_interrupt(void)
{
    lapic_write(APIC_EOI_REGISTER, 0x0);
}

/*
 * This is to start other APs via sending INIT-SIPI-SIPI sequentially
 * XXX: note the BSP may block while issuing the requests
 */
void
start_other_processors(void)
{
    LOG_INFO("booting other processors\n");
    // broadcast INIT IPIs to all other processors
    lapic_write(APIC_ICR_HIGH, 0x0);
    lapic_write(APIC_ICR_LOW, APIC_ICR_SHORTHAND_OTHER |
                              APIC_ICR_DELIVERY_INIT |
                              APIC_ICR_TRIGGER_LEVEL |
                              APIC_ICR_LEVEL_ASSERT);
    while (lapic_read(APIC_ICR_LOW) & APIC_ICR_DELIVER_STATUS_SENDPENDING);
    LOG_INFO("broadcast INIT-IPI to APs\n");
    loop_and_wait(100000000);

    // broadcast SIPIs to all orther processors
    lapic_write(APIC_ICR_HIGH, 0x0);
    lapic_write(APIC_ICR_LOW, APIC_ICR_SHORTHAND_OTHER |
                              APIC_ICR_DELIVERY_STARTUP |
                              (AP_BOOT_BASE >> 12));
    LOG_INFO("broadcast SIPI to APs\n");
    while (lapic_read(APIC_ICR_LOW) & APIC_ICR_DELIVER_STATUS_SENDPENDING);
    loop_and_wait(100000000);
}

void
local_apic_init(void)
{
    // disable master and slave pic
    outb(0xa1, 0xff);
    outb(0x21, 0xff);
    // enable local apic
    enable_local_apic();
    ASSERT(is_local_apic_enabled());
    // timer init
    lapic_write(APIC_TDC_REGISTER, 0xb);
    lapic_write(APIC_TIMER_REGISTER, APIC_TIMER_MODE_PERODIC |
                                     (IRQ_BASE + IRQ_TIMER));
    lapic_write(APIC_TIC_REGISTER, 10000000);

    //mask_lvt_entry(APIC_TIMER_REGISTER);
    // mask Local APIC timer, LINT0, LINT1, performance counter, 
    mask_lvt_entry(APIC_LINT0_REGISTER);
    mask_lvt_entry(APIC_LINT1_REGISTER);
    mask_lvt_entry(APIC_PERF_COUNTER_REGISTER);

    // map the error interrupt to IRQ_ERROR
    lapic_write(APIC_ERROR_REGISTER, IRQ_ERROR + IRQ_BASE);
    lapic_write(APIC_ERROR_STATUS_REGISTER, 0x0);
    lapic_write(APIC_ERROR_STATUS_REGISTER, 0x0);

    // acknoledge any pending interrupts
    lapic_write(APIC_EOI_REGISTER, 0x0);
    
    // enable interrup on this processor   
    lapic_write(APIC_TASK_PRIORITY_REGISTER, 0x0); 
    LOG_INFO("local apic works in legacy mode\n");
    LOG_INFO("local apic enable/disable status:%d\n", is_local_apic_enabled());
    LOG_INFO("local apic base:0x%x\n", local_apic_base());
    //start_other_processors();
}
