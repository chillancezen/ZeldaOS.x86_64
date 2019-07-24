/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/device_8253pit.h>
#include <lib64/include/string.h>
#include <lib64/include/logging.h>
#include <vm_monitor/include/vmx_pio.h>

// Document: https://wiki.osdev.org/Programmable_Interval_Timer



static void
pit_outb(uint32_t port_id, uint8_t byte, struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    switch(port_id)
    {
        case PIT_CONTROL_PORT:
            // It can only recoginze 16bit binary mode, square wave generator,
            // low/high bytes,and channel 0
            ASSERT(byte == 0x36);
            vm->pit.initial_counter = 0;
            vm->pit.divisor = 0;
            break;
        case PIT_CHANNEL0_PORT:
            if (vm->pit.initial_counter == 0) {
                vm->pit.divisor = byte & 0xff;
                vm->pit.initial_counter++;
            } else if (vm->pit.initial_counter == 1) {
                vm->pit.divisor |= (byte << 8);
                vm->pit.divisor &= 0xffff;
                vm->pit.initial_counter++;
                vm->pit.threshold =
                    1000 / (OSCILLATPR_CHIP_FREQUENCY / vm->pit.divisor);
                LOG_DEBUG("vm:0x%x(vpid:%d) pit expiry threshold: %q mseconds\n",
                          vm, vm->vpid, vm->pit.threshold);
            } else {
                PANIC_EXIT(exit);
            }
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
}

void
vmx_chipset_8253pit_preinit(void)
{
    struct portio_operation ops;
    int idx = 0;
    for (idx = 0; idx < 4; idx++) {
        memset(&ops, 0x0, sizeof(struct portio_operation));
        ops.portid = PIT_CHANNEL0_PORT + idx;
        ops.call_outb = pit_outb;
        register_pio_operation(&ops);
    }
}
