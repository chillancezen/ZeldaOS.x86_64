/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _LOGGING_H
#define _LOGGING_H
#include <lib64/include/printk.h>
#include <x86_64/include/spinlock.h>
#include <x86_64/include/panic.h>

enum log_level {
    LOG_TRIVIA = 0,
    LOG_DEBUG,
    LOG_INFO,
    LOG_WARN,
    LOG_ERROR,
    LOG_ASSERT
};

extern int __log_level;

#define LOG_TRIVIA(format, ...) {\
    if(__log_level <= LOG_TRIVIA) { \
        printk("[trivia] %s:%d "format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
}

#define LOG_TRIVIA_MP_UNSAFE(format, ...) {\
    if(__log_level <= LOG_TRIVIA) { \
        printk_mp_unsafe("[trivia] %s:%d "format, __FILE__, __LINE__, \
                         ##__VA_ARGS__); \
    } \
}

#define LOG_DEBUG(format, ...) {\
    if (__log_level <= LOG_DEBUG) { \
        printk("[debug] %s:%d "format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
}

#define LOG_DEBUG_MP_UNSAFE(format, ...) {\
    if (__log_level <= LOG_DEBUG) { \
        printk_mp_unsafe("[debug] %s:%d "format, __FILE__, __LINE__, \
                         ##__VA_ARGS__); \
    } \
}

#define LOG_INFO(format, ...) { \
    if (__log_level <= LOG_INFO) { \
        printk("[info] %s:%d "format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
}

#define LOG_INFO_MP_UNSAFE(format, ...) { \
    if (__log_level <= LOG_INFO) { \
        printk_mp_unsafe("[info] %s:%d "format, __FILE__, __LINE__, \
                         ##__VA_ARGS__); \
    } \
}

#define LOG_ERROR(format, ...) {\
    if (__log_level <= LOG_ERROR) { \
        printk("[error] %s:%d "format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
}

#define LOG_ERROR_MP_UNSAFE(format, ...) {\
    if (__log_level <= LOG_ERROR) { \
        printk_mp_unsafe("[error] %s:%d "format, __FILE__, __LINE__, \
                         ##__VA_ARGS__); \
    } \
}

#define LOG_WARN(format, ...) {\
    if (__log_level <= LOG_WARN) { \
        printk("[warn] %s:%d "format, __FILE__, __LINE__, ##__VA_ARGS__); \
    } \
}

#define LOG_WARN_MP_UNSAFE(format, ...) {\
    if (__log_level <= LOG_WARN) { \
        printk_mp_unsafe("[warn] %s:%d "format, __FILE__, __LINE__, \
                         ##__VA_ARGS__); \
    } \
}

void
__do_assertion(const char * msg, ...);

#define ASSERT(cond) {\
    if (__log_level <= LOG_ASSERT) { \
        if (!(cond)){ \
            __do_assertion("[assert] %s:%d %s failed\n", \
                           __FILE__, __LINE__, #cond); \
            __asm__ volatile("1:cli;" \
                "hlt;" \
                "jmp 1b;"); \
        }\
    } \
}

#define __NOT_REACHED 0x0
#define __not_reach() ASSERT(__NOT_REACHED)

#endif
