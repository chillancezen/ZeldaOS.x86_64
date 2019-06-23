/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <memory/include/physical_memory.h>
#include <vm_monitor/include/device_video.h>
#include <vm_monitor/include/vmx_ept.h>
#include <vm_monitor/include/vmx_exit.h>
#include <vm_monitor/include/vmx_mmio.h>
#include <lib64/include/logging.h>
#include <lib64/include/string.h>
#include <vm_monitor/include/vmx_instruction_decoding.h>
#include <device/include/video.h>
static uint64_t
video_buffer_mmio_read(uint64_t addr, int access_size,
                       struct vmexit_info * exit)
{
    uint64_t ret = 0;
    struct vmcs_blob * vm = exit->vm;
    uint64_t offset = addr - VIDEO_BUFFER_PAGE_FRAME;
    ASSERT((offset + access_size) <= PAGE_SIZE_4K);
    switch (access_size)
    {
        case ACCESS_IN_BYTE:
            ret = *(uint8_t *)(vm->regions.video_buffer + offset);
            break;
        case ACCESS_IN_WORD:
            ret = *(uint16_t *)(vm->regions.video_buffer + offset);
            break;
        case ACCESS_IN_DWORD:
            ret = *(uint32_t *)(vm->regions.video_buffer + offset);
            break;
        case ACCESS_IN_QWORD:
            ret = *(uint64_t *)(vm->regions.video_buffer + offset);
            break;
        default:
            __not_reach();
            break;
    }
    return ret;
}

static void
video_buffer_mmio_write(uint64_t addr, int access_size, uint64_t value,
                        struct vmexit_info * exit)
{
    struct vmcs_blob * vm = exit->vm;
    uint64_t offset = addr - VIDEO_BUFFER_PAGE_FRAME;
    ASSERT((offset + access_size) <= PAGE_SIZE_4K);
    switch (access_size)
    {
        case ACCESS_IN_BYTE:
            *(uint8_t *)(vm->regions.video_buffer + offset) = value;
            break;
        case ACCESS_IN_WORD:
            *(uint16_t *)(vm->regions.video_buffer + offset) = value;
            break;
        case ACCESS_IN_DWORD:
            *(uint32_t *)(vm->regions.video_buffer + offset) = value;
            break;
        case ACCESS_IN_QWORD:
            *(uint64_t *)(vm->regions.video_buffer + offset) = value;
            break;
        default:
            __not_reach();
            break;
    }
    // FIXME: only synchronize the modified bytes
    full_refresh_video(vm);
}

void
full_refresh_video(struct vmcs_blob * vm)
{
    memcpy((uint8_t *)VIDEO_BUFFER_BASE,
           (uint8_t *)vm->regions.video_buffer,
           VIDEO_BUFFER_ROW * VIDEO_BUFFER_COL * 2);
}
void
device_video_init(struct vmcs_blob * vm)
{
    setup_io_memory(vm->regions.ept_pml4_base, VIDEO_BUFFER_PAGE_FRAME);
    struct mmio_operation video_mmio = {
        .addr_low = VIDEO_BUFFER_PAGE_FRAME,
        .addr_high = VIDEO_BUFFER_PAGE_FRAME + PAGE_SIZE_4K,
        .mmio_read = video_buffer_mmio_read,
        .mmio_write = video_buffer_mmio_write
    };
    register_mmio_operation(&video_mmio);

}
