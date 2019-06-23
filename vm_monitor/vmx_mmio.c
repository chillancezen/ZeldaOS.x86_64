/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <vm_monitor/include/vmx_mmio.h>
#include <lib64/include/search.h>
#include <lib64/include/logging.h>

static struct mmio_operation  mmio_ops[MAX_NR_MMIO_REGIONS];
static int nr_mmio_ops = 0;



int
mmio_operation_compare(const struct mmio_operation * mmio1,
                       const struct mmio_operation * mmio2)
{
    // This is to realize the semantics of comparing two mmio operation region
    // if mmio1.addr_high <= mmio2.addr_low, then mmio1 < mmio2, and vice versa
    // this semantic '=' is a little different: either mmio1 region include all
    // mmio2 or mmio2 include all mmio1. any other cases are illgal
    ASSERT(mmio1->addr_low <= mmio1->addr_high);
    ASSERT(mmio2->addr_low <= mmio2->addr_high);
    if (mmio1->addr_high <= mmio2->addr_low) {
        return -1;   
    }
    if (mmio1->addr_low >= mmio2->addr_high) {
        return 1;
    }

    if ((mmio1->addr_low <= mmio2->addr_low &&
         mmio1->addr_high >= mmio2->addr_high) ||
        (mmio2->addr_low <= mmio1->addr_low &&
         mmio2->addr_high >= mmio1->addr_high)) {
        return 0;
    }
    __not_reach();
    return 0;
}

struct mmio_operation *
search_mmio_callback(uint64_t guest_pa)
{
    struct mmio_operation target = {
        .addr_low = guest_pa,
        .addr_high = guest_pa + 1
    };
    return SEARCH(struct mmio_operation, mmio_ops, nr_mmio_ops,
                  mmio_operation_compare, &target) ;
}

void
register_mmio_operation(const struct mmio_operation * mmio)
{
    ASSERT(!search_mmio_callback(mmio->addr_low))
    ASSERT(nr_mmio_ops < MAX_NR_MMIO_REGIONS);
    memcpy(&mmio_ops[nr_mmio_ops], mmio, sizeof(struct mmio_operation));
    nr_mmio_ops += 1;
    SORT(struct mmio_operation, mmio_ops, nr_mmio_ops, mmio_operation_compare);
    {
        ASSERT(search_mmio_callback(mmio->addr_low));
    }
}
