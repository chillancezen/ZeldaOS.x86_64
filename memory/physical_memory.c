/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <memory/include/physical_memory.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>


static uint64_t addr_low4G;
static uint64_t addr_high4G;
static uint64_t length_low4G;
static uint64_t length_high4G;

extern void * _kernel64_image_end;

#define PAGING_BITMAP_LENGTH_IN_BYTE \
    (MAX_MEMORY_CAPACITY_IN_GIGABYTE * 1024 * 1024 / 4 / 8)

#define PAGING_BITMAP_LENGTH_IN_PAGE (PAGING_BITMAP_LENGTH_IN_BYTE / 4096)

//static uint64_t current_avail_page;

static void
physical_page_bitmap_init(void)
{
    uint8_t * bitmap = (uint8_t *)&_kernel64_image_end;
    memset(bitmap, 0x0, PAGING_BITMAP_LENGTH_IN_BYTE);
    LOG_TRIVIA("Paging base bitmap length in byte:%q, in page:%d\n",
               PAGING_BITMAP_LENGTH_IN_BYTE, PAGING_BITMAP_LENGTH_IN_PAGE);
}

void
physical_memory_init(void)
{
    struct physical_memory_segment * seg =
        (struct physical_memory_segment *)MEMORY_LIST_LONG_MODE_BASE;
    for (; seg->length; seg = seg + 1) {
        LOG_DEBUG("address:0x%x length:0x%x type:%d\n",
                 seg->address, seg->length,
                 seg->type);
        if (seg->type != TYPE_NORMAL) {
            continue;
        }
        if (seg->address == 0x100000) {
            addr_low4G = seg->address;
            length_low4G = seg->length;
        } else if (seg->address >= 0x100000000) {
            addr_high4G = seg->address;
            length_high4G = seg->length;
        }
    }
    LOG_INFO("Detected total available memory:%q MiB\n",
             (length_low4G + length_high4G) / (1024 * 1024));
    LOG_INFO("Kernel Image End:0x%x\n", &_kernel64_image_end);
    physical_page_bitmap_init();
}

