/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _VIDEO_H
#define _VIDEO_H
#include <stdint.h>

#define VIDEO_BUFFER_BASE 0x1000b8000
#define VIDEO_BUFFER_ROW 25
#define VIDEO_BUFFER_COL 80

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


#define CHAR_VIDEO_ADDR(row, col) \
    ((uint8_t *)(uint64_t)(VIDEO_BUFFER_BASE + 2 *((row) * VIDEO_BUFFER_COL + (col))))

void
video_init(void);
#endif
