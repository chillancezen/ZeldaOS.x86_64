/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <tetris.h>
#include <video.h>
#include <lib.h>

#define _(a, b) MAKE_BYTE(a, b)
static struct shape shape0 = {   
    .nr_point = 4,
    .points = {_(0, 0), _(0, 1), _(1, 1), _(1, 2)}
};
static struct shape shape1 = {
    .nr_point = 4,
    .points = {_(0, 1), _(1, 1), _(1, 0), _(2, 0)}
};
static struct shape_group grp0 = {
    .nr_shapes = 2,
    .shapes = {&shape0, &shape1}
};


static struct shape shape2 = {
    .nr_point = 4,
    .points = {_(0, 0), _(1, 0), _(2, 0), _(3, 0)}
};
static struct shape shape3 = {
    .nr_point = 4,
    .points = {_(0, 0), _(0, 1), _(0, 2), _(0, 3)}
};
static struct shape_group grp1 = {
    .nr_shapes = 2,
    .shapes = {&shape2, &shape3}
};

static struct shape shape4 = {
    .nr_point = 4,
    .points = {_(0, 1), _(1, 0), _(1, 1), _(1, 2)}
};
static struct shape shape5 = {
    .nr_point = 4,
    .points = {_(0, 0), _(1, 0), _(1, 1), _(2, 0)}
};
static struct shape shape6 = {
    .nr_point = 4,
    .points = {_(0, 0), _(0, 1), _(0, 2), _(1, 1)}
};
static struct shape shape7 = {
    .nr_point = 4,
    .points = {_(0, 1), _(1, 0), _(1, 1), _(2, 1)}
};
static struct shape_group grp2 = {
    .nr_shapes = 4,
    .shapes = {&shape4, &shape5, &shape6, &shape7}
};

static struct shape shape8 = {
    .nr_point = 4,
    .points = {_(0, 0), _(0, 1), _(1, 0), _(1, 1)}
};
static struct shape_group grp3 = {
    .nr_shapes = 1,
    .shapes = {&shape8}
};

#undef _

__attribute__((unused))
static struct shape_group * shape_grps[NR_SHAPE_TYPE] = {
    &grp0, &grp1, &grp2, &grp3
};

static struct grid_elem grid[GRID_HEIGHT][GRID_WIDTH];
static struct block blk;
static int is_running = 0;
static int started = 0;
static int score;
static void
plot_shape(const struct shape *shp, int color, int x_pos, int y_pos)
{
    int x_base = x_pos + GRID_AXIS_X;
    int y_base = y_pos + GRID_ASIS_Y;
    int idx = 0;
    for (idx = 0; idx < shp->nr_point; idx++) {
        uint8_t x_offset = LOW_NIBBLE(shp->points[idx]);
        uint8_t y_offset = HIGH_NIBBLE(shp->points[idx]);
        uint8_t * ptr = CHAR_VIDEO_ADDR(y_base + y_offset, x_base + x_offset);
        ptr[0] = ' ';
        ptr[1] = MAKE_BYTE(color, COLOR_WHITE);
    }
}


static void
unplot_shape(const struct shape *shp, int x_pos, int y_pos)
{
    int x_base = x_pos + GRID_AXIS_X;
    int y_base = y_pos + GRID_ASIS_Y;
    int idx = 0;
    for (idx = 0; idx < shp->nr_point; idx++) {
        uint8_t x_offset = LOW_NIBBLE(shp->points[idx]);
        uint8_t y_offset = HIGH_NIBBLE(shp->points[idx]);
        uint8_t * ptr = CHAR_VIDEO_ADDR(y_base + y_offset, x_base + x_offset);
        ptr[0] = '.';
        ptr[1] = MAKE_BYTE(COLOR_BLACK, COLOR_BLUE);
    }
}
void
plot_block(struct block * blk)
{
    plot_shape(blk->shp, blk->color, blk->x_pos, blk->y_pos);
    blk->plotted = 1;
}

void
unplot_block(struct block * blk)
{
    if (!blk->plotted) {
        return;
    }
    unplot_shape(blk->shp, blk->x_pos, blk->y_pos);
    blk->plotted = 0;
}

void
rotate_block(struct block * blk)
{
    int current_index = -1;
    int next_index = 0;
    int idx = 0;
    for (idx = 0; idx < blk->grp->nr_shapes; idx++) {
        if (blk->grp->shapes[idx] == blk->shp) {
            current_index = idx;
            break;
        }
    }
    if (current_index < 0) {
        next_index = 0;
    } else {
        next_index = current_index + 1;
        next_index = next_index % blk->grp->nr_shapes;
    }
    blk->shp = blk->grp->shapes[next_index];
}

int
is_block_clashing(const struct block * blk)
{
    int idx = 0;
    for (; idx < blk->shp->nr_point; idx++) {
        uint8_t x_offset = LOW_NIBBLE(blk->shp->points[idx]);
        uint8_t y_offset = HIGH_NIBBLE(blk->shp->points[idx]);
        int x_pos = blk->x_pos + x_offset;
        int y_pos = blk->y_pos + y_offset;
        if (x_pos <0 || x_pos >= GRID_WIDTH ||
            y_pos <0 || y_pos >= GRID_HEIGHT) {
            return 1;
        }
        if (grid[y_pos][x_pos].is_set) {
            return 1;
        }
    }
    return 0; 
}

void
finalize_block(const struct block * blk)
{
    int idx = 0;
    for (; idx < blk->shp->nr_point; idx++) {
        uint8_t x_offset = LOW_NIBBLE(blk->shp->points[idx]);
        uint8_t y_offset = HIGH_NIBBLE(blk->shp->points[idx]);
        int x_pos = blk->x_pos + x_offset;
        int y_pos = blk->y_pos + y_offset;
        grid[y_pos][x_pos].is_set = 1;
        grid[y_pos][x_pos].color = blk->color;
    }
}

void
purge_grid(void)
{
    int rows_left_iptr = 0;
    uint8_t rows_left[GRID_HEIGHT];
    memset(rows_left, 0x0, sizeof(rows_left));
    int idx = 0, idx_another;
    int purged_count = 0;
    for (idx = GRID_HEIGHT - 1; idx >= 0; idx--) {
        for (idx_another = 0; idx_another < GRID_WIDTH; idx_another++) {
            if (!grid[idx][idx_another].is_set) {
                rows_left[rows_left_iptr++] = idx;
                break;
            }
        }
        if (idx_another == GRID_WIDTH) {
            purged_count++;
        }
    }
    if (!purged_count) {
        return;
    }
    score += purged_count;
    printk("current score:%d\n", score);
    int target_row = GRID_HEIGHT - 1;
    for (idx = 0; idx < rows_left_iptr; idx++) {
        int source_row = rows_left[idx];
        for (idx_another = 0; idx_another < GRID_WIDTH; idx_another++) {
            grid[target_row][idx_another] = grid[source_row][idx_another];
        }
        target_row--;
    }
    while (target_row >= 0) {
        for (idx_another = 0; idx_another < GRID_WIDTH; idx_another++) {
            grid[target_row][idx_another].is_set = 0;
        }
        target_row--;
    }
}

static int colors[] = {COLOR_RED, COLOR_PURPLE, COLOR_GRAY, COLOR_CYAN,
                       COLOR_YELLOW, COLOR_BROWN};
static void
new_block(struct block * blk)
{
    int grp_index = rand(NR_SHAPE_TYPE);
    int shp_index = rand(shape_grps[grp_index]->nr_shapes);
    int color_index = rand(sizeof(colors) / sizeof(int));
    blk->grp = shape_grps[grp_index];
    blk->shp = shape_grps[grp_index]->shapes[shp_index];
    blk->color = colors[color_index];
    blk->x_pos = GRID_WIDTH / 2;
    blk->y_pos = 0;
}
void
on_key_blank(void)
{
    if (!is_running) {
        return; 
    }
    struct block tmp = blk;
    rotate_block(&tmp);
    if (!is_block_clashing(&tmp)) {
        unplot_block(&blk);
        rotate_block(&blk);
        plot_block(&blk);
    }
}

static void
init_grid(int inner_frame_only);

void
on_key_arrow_down(void)
{
    if (!is_running) {
        return;
    }
    struct block tmp = blk;
    tmp.y_pos++;
    if (is_block_clashing(&tmp)) {
        finalize_block(&blk);
        purge_grid();
        init_grid(1);
        new_block(&blk);
        if (is_block_clashing(&blk)) {
            is_running = 0;
            printk("teris running:%s\n", is_running ? "true" : "false");
        } else {
            plot_block(&blk);
        }
    } else {
        unplot_block(&blk);
        blk.y_pos++;
        plot_block(&blk);
    }
}

void
on_key_arrow_left(void)
{
    if (!is_running) {
        return;
    }
    struct block tmp = blk;
    tmp.x_pos--;
    if (!is_block_clashing(&tmp)) {
        unplot_block(&blk);
        blk.x_pos--;
        plot_block(&blk);
    }
}

void
on_key_arrow_right(void)
{
    if (!is_running) {
        return;
    }
    struct block tmp = blk;
    tmp.x_pos++;
    if (!is_block_clashing(&tmp)) {
        unplot_block(&blk);
        blk.x_pos++;
        plot_block(&blk);
    }
}
static void
init_grid(int inner_frame_only)
{
    int x, y;
    for (y = 0; y < GRID_HEIGHT; y++) {
        for (x = 0; x < GRID_WIDTH; x++) {
            int pos_x = x + GRID_AXIS_X;
            int pos_y = y + GRID_ASIS_Y;
            uint8_t * ptr = CHAR_VIDEO_ADDR(pos_y, pos_x);
            ptr[0] = grid[y][x].is_set ? ' ' : '.';
            ptr[1] = grid[y][x].is_set ?
                     MAKE_BYTE(grid[y][x].color, COLOR_BLUE) :
                     MAKE_BYTE(COLOR_BLACK, COLOR_BLUE);
        }
    }
    if (inner_frame_only) {
        return;
    }
    for (x = GRID_AXIS_X - 1; x < (GRID_AXIS_X + GRID_WIDTH + 1); x++) {
        uint8_t * ptr = CHAR_VIDEO_ADDR(24, x);
        ptr[0] = '=';
        ptr[1] = MAKE_BYTE(COLOR_BLACK, COLOR_WHITE);
    }
    for (y = GRID_ASIS_Y; y < (GRID_ASIS_Y + GRID_HEIGHT); y++) {
        uint8_t * ptr = CHAR_VIDEO_ADDR(y, GRID_AXIS_X -1);
        ptr[0] = '=';
        ptr[1] = MAKE_BYTE(COLOR_BLACK, COLOR_WHITE);

        ptr = CHAR_VIDEO_ADDR(y, GRID_AXIS_X + GRID_WIDTH);
        ptr[0] = '=';
        ptr[1] = MAKE_BYTE(COLOR_BLACK, COLOR_WHITE);
    }
}

void
tetris_start(void)
{
    if (!started) {
        video_remap();
        init_grid(0);
        new_block(&blk);

        started = 1;
    }
    is_running = !is_running;
    printk("teris running:%s\n", is_running ? "true" : "false");
}

void
tetris_reset(void)
{
    if (!started) {
        return;
    }
    memset(grid, 0x0, sizeof(grid));
    init_grid(0);
    new_block(&blk);
    is_running = 1;
}
void
tetris_init(void)
{
    memset(grid, 0x0, sizeof(grid));
    memset(&blk, 0x0, sizeof(blk));
    print_text_with_color(19, 15, "Press [Enter] to start the Demo: {Game: TETRIS}",
                          MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(15, 15, "VM STATE     :   READY", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(16, 15, "Hypervisor   :   ZeldaOS", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    print_text_with_color(17, 15, "Acrhitecture :   x86_64", MAKE_BYTE(COLOR_BLACK, COLOR_YELLOW));
    //init_grid(0);
    //new_block(&blk);
    //plot_block(&blk);
}
