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

static struct portio_operation *
search_portio_callback(uint32_t port_id)
{
    struct portio_operation target = {
        .portid = port_id
    };
    return SEARCH(struct portio_operation, portio_ops, nr_ports,
                  port_ops_compare, &target);
}

void
register_pio_operation(const struct portio_operation * pio)
{
    struct portio_operation * pio_found = search_portio_callback(pio->portid);
    if (pio_found) {
        ASSERT(pio_found->portid == pio->portid);
        memcpy(pio_found, pio, sizeof(struct portio_operation));
        return;
    }
    ASSERT(nr_ports < MAX_PORTS_SUPPORTED);
    memcpy(&portio_ops[nr_ports++], pio, sizeof(struct portio_operation));
    SORT(struct portio_operation, portio_ops, nr_ports, port_ops_compare);
    ASSERT((pio_found = search_portio_callback(pio->portid)) &&
           pio_found->portid == pio->portid);
}

uint64_t
io_instruction_exit_sub_handler(struct vmexit_info * exit)
{
    // See Table 27-5. Exit Qualification for I/O Instructions
    // here we enforce lots of limitations here:
    //      1) MUST NOT be be string IO
    //      2) MUST NOT be REP prefixed
    //      3) The operand encoding MUST be %DX register, MUST NOT be immediate
    uint8_t access_size = exit->exit_qualification & 7;
    uint8_t access_direc = (exit->exit_qualification >> 3) & 1;
    uint16_t port_id = (exit->exit_qualification >> 16) & 0xffff;
    struct portio_operation *ops;
    ASSERT(!((exit->exit_qualification >> 4) & 1));
    ASSERT(!((exit->exit_qualification >> 5) & 1));
    ASSERT(!((exit->exit_qualification >> 6) & 1));
    ASSERT((ops = search_portio_callback(port_id)));
    switch(access_size)
    {
        case 0:// access in byte
            if (access_direc == 0) {
                ASSERT(ops->call_outb);
                ops->call_outb(port_id, exit->vm->vcpu->rax, exit);
            } else {
                ASSERT(ops->call_inb);
                exit->vm->vcpu->rax = ops->call_inb(port_id, exit);
            }
            break;
        default:
            PANIC_EXIT(exit);
            break;
    }
    GOTO_NEXT_INSTRUCTION(exit);
    return (uint64_t)exit->vm->vcpu;
}


__attribute__((constructor)) static void
portio_preinit(void)
{
    nr_ports = 0;
    memset(portio_ops, 0x0, sizeof(portio_ops));
}
