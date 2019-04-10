/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <x86_64/include/ioapic.h>
#include <lib64/include/logging.h>
#include <memory/include/paging.h>
#include <x86_64/include/ioport.h>

static void
ioapic_write(uint32_t offset, uint32_t value)
{
    *(volatile uint32_t *)(IOAPIC_BASE) = offset;
    *(volatile uint32_t *)(IOAPIC_BASE + 0x10) = value;
}


static uint32_t
ioapic_read(uint32_t offset)
{
    *(volatile uint32_t *)(IOAPIC_BASE) = offset;
    return *(volatile uint32_t *)(IOAPIC_BASE + 0x10);
}

/*
 * This helper function enable the redirection entry at the specific postion
 * and redirect the interrupt to the cpu
 * this redirection entry will be unmasked
 */
void
ioapic_redirect_entry(uint32_t entry, uint32_t cpu)
{
    ioapic_write(IOAPIC_OFFSET_RED_TBL(entry), 0x20 + entry);
    ioapic_write(IOAPIC_OFFSET_RED_TBL(entry) + 1, cpu << 24);
}

void
io_apic_init(void)
{
    // Map the ioapic configuration page
    ASSERT(map_address(IOAPIC_BASE, IOAPIC_BASE, PAGE_SIZE_4K) == ERROR_OK);
    uint32_t ioapic_id = ioapic_read(IOAPIC_OFFSET_ID);
    uint32_t ioapic_version = ioapic_read(IOAPIC_OFFSET_VERSION);
    LOG_INFO("IOAPIC ID:0x%x\n", ioapic_id);
    LOG_INFO("IOAPIC:%x's maximum interrupts supported: 0x%x\n",
             ioapic_id, (ioapic_version >> 16) + 1);
    // Mask all the redirection entry, redirect the interrupt to cpu 0
    // by default
    int idx = 0;
    for (idx = 0; idx <= (ioapic_version >> 16); idx++) {
        ioapic_write(IOAPIC_OFFSET_RED_TBL(idx), IOAPIC_REDIRECT_ENTRY_MASKED |
                                                 (0x20 + idx));
        ioapic_write(IOAPIC_OFFSET_RED_TBL(idx) + 1, 0);
    }
}

