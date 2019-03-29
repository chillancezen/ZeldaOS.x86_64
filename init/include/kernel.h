/*
 * Copyright (c) 2019 Jie Zheng
 */

#ifndef _KERNEL_H
#define _KERNEL_H

#define halt() {\
    __asm__ volatile ("1:cli;" \
        "hlt;" \
        "jmp 1b;");\
}

#endif
