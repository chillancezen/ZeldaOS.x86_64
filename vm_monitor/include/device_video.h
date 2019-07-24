/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _DEVICE_VIDEO_H
#define _DEVICE_VIDEO_H
#include <vm_monitor/include/vmx_vmcs.h>

#define VIDEO_BUFFER_PAGE_FRAME 0x1000b8000

void
full_refresh_video(struct vmcs_blob * vm);

void
incremental_refresh_video(struct vmcs_blob * vm, int offset, int size);

void
device_video_init(struct vmcs_blob * vm);
#endif
