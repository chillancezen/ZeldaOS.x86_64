/*
 * Copyright (c) 2019 Jie Zheng
 * 
 * The Stage II code of boot loader
 */

char * str[512];


void
stage2_main(void)
{
    asm __volatile(
        "movl $0xb8000, %edx;"
        "movb $0x49, (%edx);"
        "1:"
        "cli;"
        "hlt;"
        "jmp 1b;");

}
