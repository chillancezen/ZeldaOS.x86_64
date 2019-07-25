/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _TETRIS_H
#define _TETRIS_H
#include <stdint.h>

#define MAX_BLOCK_WIDTH 4
#define MAX_BLOCK_HEIGHT 4


#define GRID_WIDTH 16
#define GRID_HEIGHT 20
#define GRID_AXIS_X ((80 - GRID_WIDTH) / 2)
#define GRID_ASIS_Y 4

#define MAKE_BYTE(hi, lo) ((((hi) << 4) & 0xf0) | ((lo) & 0xf))
#define LOW_NIBBLE(by) ((by) & 0xf)
#define HIGH_NIBBLE(by) (((by) >> 4) & 0xf)


#define MAX_POINTS_IN_SHAPE 8

struct shape {
    int nr_point;
    uint8_t points[MAX_POINTS_IN_SHAPE];
};
#define MAX_SHAPES_IN_GROUP 4

struct shape_group {
    int nr_shapes;
    struct shape * shapes[MAX_SHAPES_IN_GROUP];
};

#define NR_SHAPE_TYPE 4

struct block {
    int x_pos;
    int y_pos;
    int color;
    struct shape * shp;
    struct shape_group * grp;
};

struct grid_elem {
    uint8_t is_set;
    uint8_t color;
};
enum DIRECTION {
    DIR_LEFT,
    DIR_DOWN,
    DIR_RIGHT
};
void
tetris_init(void);

void
on_key_blank(void);

void
on_key_arrow_down(void);

void
on_key_arrow_left(void);

void
on_key_arrow_right(void);
#endif
