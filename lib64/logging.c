/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <lib64/include/logging.h>
#include <stdarg.h>
#include <device/include/video.h>
#include <lib64/include/string.h>
int __log_level = DEFAULT_LOGGING_LEVEL;

struct spinlock __logging_lock;

static void
draw_psod_framework(void)
{
    int idx = 0;
    for (idx = 0; idx < VIDEO_BUFFER_COL; idx++) {
        *CHAR_VIDEO_ADDR(0, idx) = '=';
        *CHAR_VIDEO_ADDR(VIDEO_BUFFER_ROW - 1, idx) = '=';
    }
}
void
__do_assertion(const char * fmt, ...)
{
    va_list args;
    struct auxiliary_state64 aux_stat;
    va_start(args, fmt);
    printk_mp_raw(fmt, args);
    read_auxiliary_state(&aux_stat);
    dump_auxiliary_state(&aux_stat);
    backtrace();
    // Make the purple screen of death(PSOD from VMware ESXi)
    reset_text();
    set_font_color(COLOR_PURPLE, COLOR_WHITE);
    draw_psod_framework();
    print_text(1, 0, "Copyright (c) 2019 Jie Zheng");
    print_text(2, 0, "https://github.com/chillancezen/ZeldaOS.x86_64");

    int row_index = 4;
    char line[128];
    va_list args_another;
    va_start(args_another, fmt);
    sprintf_raw(line, fmt, args_another);
    {
        char * ptr = line;
        for(; *ptr; ptr++) {
            if (*ptr == '\n') {
                *ptr = '\0';
                break;
            }
        }
    }
    print_text(row_index++, 0, line);

    sprintf(line, "%s:%d ""CPU:%d IDT.BASE:0x%x IDT.SIZE:0x%x",
            __FILE__, __LINE__,
            aux_stat.cpu, aux_stat.idt_base, aux_stat.idt_size);
    print_text(row_index++, 0, line);

    sprintf(line, "%s:%d ""TR:0x%x, GDT.BASE:0x%x, GDT.SIZE:0x%x",
            __FILE__, __LINE__,
            aux_stat.TR, aux_stat.gdt_base, aux_stat.gdt_size);
    print_text(row_index++, 0, line);

    sprintf(line, "%s:%d ""FS.BASE:0x%x, GS.BASE:0x%x, EFER:0x%x",
            __FILE__, __LINE__,
            aux_stat.FS_BASE_MSR, aux_stat.GS_BASE_MSR, aux_stat.EFER_MSR);
    print_text(row_index++, 0, line);

    sprintf(line, "%s:%d ""CR4:0x%x, CR3:0x%x, CR2:0x%x, CR0:0x%x",
            __FILE__, __LINE__,
            aux_stat.cr4, aux_stat.cr3, aux_stat.cr2, aux_stat.cr0);
    print_text(row_index++, 0, line);

    struct backtrace_blob * blob = get_backtrace_blob();
    int idx = 0;
    for(idx = 0; idx < blob->nr_frames; idx++) { 
        sprintf(line, "%s:%d ""frame #%d <%x>",
                __FILE__, __LINE__,
                idx, blob->frames[idx]);
        print_text(row_index++, 0, line);
    }
    va_end(args_another);
    va_end(args);
}

__attribute__((constructor)) void
logging_pre_init(void)
{
    spinlock_init(&__logging_lock);
}


