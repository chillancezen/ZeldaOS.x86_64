/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <video.h>


void
video_init(void)
{
    *(uint8_t *)VIDEO_BUFFER_BASE = 'a';
    *(uint8_t *)(VIDEO_BUFFER_BASE + 1) =0x0f;
}
