/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _SORT_H
#define _SORT_H
#include <lib64/include/type.h>
#include <lib64/include/string.h>

#define ELEM(type, base, index) ((index)+ (type *)((uint64_t)(base)))

// Here I use the simplest bubble sort in an ascending order
#define SORT(type, base, num, comp) {                                          \
    int idx_out = (num) - 1;                                                   \
    for (; idx_out > 0; idx_out--) {                                           \
        int idx_in = 0;                                                        \
        for (; idx_in < idx_out; idx_in++) {                                   \
            type * prev_elem = ELEM(type, base, idx_in);                       \
            type * next_elem = ELEM(type, base, idx_in + 1);                   \
            if (comp(prev_elem, next_elem) > 0) {                              \
                type tmp_elem;                                                 \
                memcpy(&tmp_elem, prev_elem, sizeof(type));                    \
                memcpy(prev_elem, next_elem, sizeof(type));                    \
                memcpy(next_elem, &tmp_elem, sizeof(type));                    \
            }                                                                  \
        }                                                                      \
    }                                                                          \
}


#endif
