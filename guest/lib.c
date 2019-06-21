/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <lib.h>
#include <serial.h>

void
memset(void *dst, const void *src, int nr_bytes)
{
    int idx = 0;
    for (; idx < nr_bytes; idx++) {
        *(idx + (uint8_t *)(dst)) = *(idx + (uint8_t *)(src));
    }
}

void
print_string(const char * content)
{
    int idx = 0;
    while (content[idx]) {
        write_serial(content[idx]);
        idx++;
    }
}
