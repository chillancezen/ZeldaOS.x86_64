/*
 * Copyright (c) 2019 Jie Zheng
 */
#include <serial.h>

void
print_string(const char * context)
{
    int idx = 0;
    while (context[idx]) {
        write_serial(context[idx]);
        idx++;
    }
}
void
guest_kernel_main(void)
{
    print_string("Hello World from Guest\n");

    //*(uint64_t *)0xffb8000 = 0x1234;
}
