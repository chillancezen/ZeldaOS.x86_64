/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _VIDEO_H
#define _VIDEO_H

#include <lib64/include/type.h>

#define COLOR_BLACK 0x0
#define COLOR_BLUE 0x1
#define COLOR_GREEN 0x2
#define COLOR_CYAN 0x3
#define COLOR_RED 0x4
#define COLOR_PURPLE 0x5
#define COLOR_BROWN 0x6
#define COLOR_GRAY 0x7
#define COLOR_DARK_GRAY 0x8
#define COLOR_LIGHT_BLUE 0x9
#define COLOR_LIGHT_GREEN 0xa
#define COLOR_LIGHT_CYAN 0xb
#define COLOR_LIGHT_RED 0xc
#define COLOR_LIGHT_PURPLE 0xd
#define COLOR_YELLOW 0xe
#define COLOR_WHITE 0xf

#define VIDEO_BUFFER_BASE 0xb8000
#define VIDEO_BUFFER_ROW 25
#define VIDEO_BUFFER_COL 80


#define CHAR_VIDEO_ADDR(row, col) \
    ((char *)(uint64_t)(VIDEO_BUFFER_BASE + 2 *((row) * VIDEO_BUFFER_COL + (col))))

void
set_font_color(int background_color, int front_color);

void
reset_text(void);

void
print_text(int idx_row, int idx_col, const char * text);

#endif
