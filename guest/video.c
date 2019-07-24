/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <video.h>


void
video_init(void)
{
    *(uint8_t *)VIDEO_BUFFER_BASE = ' ';
    *(uint8_t *)(VIDEO_BUFFER_BASE + 1) =0xf0;
    *(uint8_t *)(VIDEO_BUFFER_BASE + 2) =' ';
    *(uint8_t *)(VIDEO_BUFFER_BASE + 3) =0xf0;
    *(uint8_t *)(VIDEO_BUFFER_BASE + 2 + 80 * 2) =' ';
    *(uint8_t *)(VIDEO_BUFFER_BASE + 3 + 80 * 2) =0xf0;
    *(uint8_t *)(VIDEO_BUFFER_BASE + 4 + 80 * 2) =' ';
    *(uint8_t *)(VIDEO_BUFFER_BASE + 5 + 80 * 2) =0xf0;
}
