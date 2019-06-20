/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_exit.h>
#include <lib64/include/logging.h>
#include <lib64/include/sort.h>
#include <lib64/include/search.h>
#include <vm_monitor/include/vmx_pio.h>

static struct portio_operation portio_ops[MAX_PORTS_SUPPORTED];
static int nr_ports = 0;

static int
port_ops_compare(const struct portio_operation * port1,
                 const struct portio_operation * port2)
{
    return port1->portid - port2->portid;
}

struct portio_operation *
search_portio_callback(uint32_t port_id)
{
    struct portio_operation target = {
        .portid = port_id
    };
    return SEARCH(struct portio_operation, portio_ops, nr_ports,
                  port_ops_compare, &target);
}

uint64_t
io_instruction_exit_sub_handler(struct vmexit_info * exit)
{
    GOTO_NEXT_INSTRUCTION(exit);
    PANIC_EXIT(exit);
    return (uint64_t)exit->vm->vcpu;
}


__attribute__((constructor)) static void
portio_preinit(void)
{
    nr_ports = 0;
    memset(portio_ops, 0x0, sizeof(portio_ops));
}
