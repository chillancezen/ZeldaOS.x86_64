/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <device/include/video.h>
#include <x86_64/include/ioport.h>


static void
disable_cursor(void)
{
    outb(0x3d4, 0x0a);
    outb(0x3d5, 0x20);
}

void
set_font_color(int background_color, int front_color)
{
    int idx_row, idx_col;
    for (idx_row = 0; idx_row < VIDEO_BUFFER_ROW; idx_row++) {
        for (idx_col = 0; idx_col < VIDEO_BUFFER_COL; idx_col++) {
            uint8_t * ptr = (uint8_t *)(uint64_t)(VIDEO_BUFFER_BASE +
                                                  idx_row * VIDEO_BUFFER_COL * 2
                                                  + idx_col * 2);
            ptr[1] = background_color << 4 | front_color;
        }
    }
}

void
reset_text(void)
{
    int idx_row, idx_col;
    for (idx_row = 0; idx_row < VIDEO_BUFFER_ROW; idx_row++) {
        for (idx_col = 0; idx_col < VIDEO_BUFFER_COL; idx_col++) {
            uint8_t * ptr = (uint8_t *)(uint64_t)(VIDEO_BUFFER_BASE +
                                                  idx_row * VIDEO_BUFFER_COL * 2
                                                  + idx_col * 2);
            ptr[0] = 0x0;
        }
    }
}

void
print_text(int idx_row, int idx_col, const char * text)
{
    uint8_t * ptr = (uint8_t *)(uint64_t)(VIDEO_BUFFER_BASE +
                                          idx_row * VIDEO_BUFFER_COL * 2 +
                                          idx_col * 2);
    const char * text_ptr = text;
    for (; *text_ptr; text_ptr += 1, ptr += 2) {
        *ptr = *text_ptr;
    }
}
static const char *banner[] ={    
"________________________________________________",
"     ___                               __       ",
"        /          /       /         /    )     ",
"-------/-----__---/----__-/----__---/----/---__-",
"      /    /___) /   /   /   /   ) /    /   (_ `",
"____(_____(___ _/___(___/___(___(_(____/___(__)_",
"    /                                           ",
"(_ /   Copyright (c) 2019 Jie Zheng             ",
"",
"https://github.com/chillancezen/ZeldaOS.x86_64"
};
__attribute__((constructor)) void
video_init(void)
{
    disable_cursor();
    set_font_color(COLOR_BLACK, COLOR_GREEN);
    reset_text();
    
    {
        // print the banner
        int idx = 0;
        for (idx = 0; idx < (sizeof(banner) / sizeof(char *)); idx++) {
            print_text(idx + 4, 15, banner[idx]);
        }
    }
}
