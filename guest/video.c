/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <video.h>
#include <tetris.h>
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

void
print_text_with_color(int idx_row, int idx_col, const char * text, int color)
{
    uint8_t * ptr = (uint8_t *)(uint64_t)(VIDEO_BUFFER_BASE +
                                          idx_row * VIDEO_BUFFER_COL * 2 +
                                          idx_col * 2);
    const char * text_ptr = text;
    for (; *text_ptr; text_ptr += 1, ptr += 2) {
        *ptr = *text_ptr;
        ptr[1] = color;
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
"https://github.com/chillancezen/ZeldaOS.x86_64",
"================================================"
};

void
video_remap(void)
{
    set_font_color(COLOR_BLACK, COLOR_GREEN);
    reset_text();

    {
        // print the banner
        int idx = 0;
        for (idx = 0; idx < (sizeof(banner) / sizeof(char *)); idx++) {
            print_text(idx, 0, banner[idx]);
        }
    }
    print_text_with_color(15, 2, "Instructions:", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(16, 2, "  [ Enter ] suspend/resume the game", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(17, 2, "  [ Ctrl+R] reset the game", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(18, 2, "  [ blank]  rotate the shape", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(19, 2, "  [ down arrow ]  adjust the axis Y position by +1", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(20, 2, "  [ left arrow ]  adjust the axis X position by -1", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(21, 2, "  [ right arrow]  adjust the axis X position by +1", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
}

void
video_init(void)
{
}
