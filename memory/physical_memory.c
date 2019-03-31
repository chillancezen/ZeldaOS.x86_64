/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <memory/include/physical_memory.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>


static uint64_t addr_low4G = 0;
static uint64_t addr_high4G = 0;
static uint64_t length_low4G = 0;
static uint64_t length_high4G = 0;

extern void * _kernel64_image_end;

#define PAGING_BITMAP_LENGTH_IN_BYTE \
    (MAX_MEMORY_CAPACITY_IN_GIGABYTE * 1024 * 1024 / 4 / 8)

#define PAGING_BITMAP_LENGTH_IN_PAGE (PAGING_BITMAP_LENGTH_IN_BYTE / 4096)

static uint64_t current_avail_page;

__attribute__((always_inline)) static inline uint64_t
address_to_byte_index(uint64_t addr)
{
    return addr >> 12 >> 3;
}

__attribute__((always_inline)) static inline uint8_t
address_to_bit_index(uint64_t addr)
{
    return ((addr >> 12) & 0x7);
}

static uint8_t
is_page_free(uint64_t addr)
{
    uint8_t * bitmap = (uint8_t *)&_kernel64_image_end;
    uint64_t byte_index = address_to_byte_index(addr);
    uint8_t bit_index = address_to_bit_index(addr);
    if (byte_index >= PAGING_BITMAP_LENGTH_IN_BYTE) {
        // Not a valid paging index at all, it's beyond the address scope we
        // are going to handle
        return 0;
    }
    return !((bitmap[byte_index] >> bit_index) & 0x1);
}

static void
mark_page_as_occupied(uint64_t addr)
{
    uint8_t * bitmap = (uint8_t *)&_kernel64_image_end;
    uint64_t byte_index = address_to_byte_index(addr);
    uint8_t bit_index = address_to_bit_index(addr);

    bitmap[byte_index] |= 1 << bit_index;
}

static void
physical_page_bitmap_init(void)
{
    uint8_t * bitmap = (uint8_t *)&_kernel64_image_end;
    memset(bitmap, 0x0, PAGING_BITMAP_LENGTH_IN_BYTE);
    LOG_TRIVIA("Paging base bitmap length in byte:%q, in page:%d\n",
               PAGING_BITMAP_LENGTH_IN_BYTE, PAGING_BITMAP_LENGTH_IN_PAGE);
    // Marks all pages below the end of base pages
    {
        uint64_t base_pages_end = PAGING_BITMAP_LENGTH_IN_PAGE * PAGE_SIZE_4K +
                                  (uint64_t)&_kernel64_image_end;
        uint64_t page = 0;
        for (; page < base_pages_end; page += PAGE_SIZE_4K) {
            mark_page_as_occupied(page);
            ASSERT(!is_page_free(page));
        }
        current_avail_page = base_pages_end + PAGE_SIZE_4K;
    }
}
/*
 * XXX:To simply the management of physical memory, I do not reclaim these pages
 * as I do not realize a full fledged kernel as my 32-bit ZeldaOS did.that means
 * the bitmap indicator is not used at all.
 * XXX: The allocated pages can not breach the 3G barrier.
 * This is to allocate raw pages from system-wide physical memory
 * @return the physical 4K aligned address, 0 is returned upon failure
 */
#define PHYSICAL_PAGE_VALID(addr) \
    (((addr) >= addr_low4G && (addr) < (addr_low4G + length_low4G)) || \
    ((addr) >= addr_high4G && (addr) < (addr_high4G + length_high4G)))
uint64_t
get_physical_pages(int nr_pages)
{
    uint64_t next_page;
    uint64_t first_page = current_avail_page;
    uint64_t last_page = current_avail_page + PAGE_SIZE_4K * (nr_pages -1);
    ASSERT(nr_pages > 0);
    if (!PHYSICAL_PAGE_VALID(first_page) || !PHYSICAL_PAGE_VALID(last_page)) {
        return 0;
    }
    next_page = current_avail_page + PAGE_SIZE_4K * nr_pages;
    if (!PHYSICAL_PAGE_VALID(next_page) && next_page < addr_high4G) {
        next_page = addr_high4G;
    }
    current_avail_page = next_page;
    return first_page;
}

uint64_t
get_physical_page(void)
{
    return get_physical_pages(1);
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

