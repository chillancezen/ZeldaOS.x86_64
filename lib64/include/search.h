/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _SEARCH_H
#define _SEARCH_H
#include <lib64/include/string.h>
#include <lib64/include/sort.h>

// this is to search the target in a binary way.
#define SEARCH(type, base, num, comp, target) ({                               \
    type * elem_found = (void *)0;                                             \
    int idx_low = 0;                                                           \
    int idx_high = (num) - 1;                                                  \
    for (; idx_low <= idx_high;) {                                             \
        type * elem_low = ELEM(type, base, idx_low);                           \
        type * elem_high = ELEM(type, base, idx_high);                         \
        if (!comp(target, elem_low)) {                                         \
            elem_found = elem_low;                                             \
            break;                                                             \
        } else if (!comp(target, elem_high)) {                                 \
            elem_found = elem_high;                                            \
            break;                                                             \
        } else {                                                               \
            int idx_mid = (idx_low + idx_high) / 2;                            \
            if (idx_mid == idx_low || idx_mid == idx_high) {                   \
                break;                                                         \
            }                                                                  \
            type * elem_mid = ELEM(type, base, idx_mid);                       \
            if (comp(target, elem_mid) < 0) {                                  \
                idx_high = idx_mid;                                            \
            } else {                                                           \
                idx_low = idx_mid;                                             \
            }                                                                  \
        }                                                                      \
    }                                                                          \
    elem_found;                                                                \
})
#endif
