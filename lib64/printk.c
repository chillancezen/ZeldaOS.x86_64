/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <lib64/include/printk.h>

#define TEXT_VIDEO_BASE 0xb8000

//static int vga_row_front = 0;
//static int vga_row_idx = 0;
//static int vga_col_idx = 0;

//static uint8_t vga_shadow_memory[VGA_MAX_ROW+1][VGA_MAX_COL];
//static uint16_t (*vga_ptr)[VGA_MAX_COL] = (void *)VGA_MEMORY_BASE;


void
printk(const char * fmt, ...)
{

}

__attribute__((constructor)) static void
printk_pre_init(void)
{

}
