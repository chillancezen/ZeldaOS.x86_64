/*
 * Copyright (c) 2019 Jie Zheng
 *
 * The serial port com0 management:
 *      1). the raw write interface (*)
 *      2). the higher level abstraction
 */
#include <x86_64/include/ioport.h>

#define COM1_PORT 0x3f8

static int
is_transmit_empty(void)
{
    return inb(COM1_PORT + 5) & 0x20;
}

static void
write_serial(uint8_t a)
{
    while (is_transmit_empty() == 0);   
        outb(COM1_PORT,a);
}

void
print_string(char * str)
{
    while(*str)
        write_serial(*str++);
}

void
print_hexdecimal(void * val)
{
    int idx = 0;
    uint8_t dict[] = "0123456789abcdef";
    uint8_t * ptr = (uint8_t *)&val;

    for(idx = 0, ptr += 3; idx < 4; idx++, ptr--) {
        write_serial(dict[((*ptr) >> 4) & 0xf]);
        write_serial(dict[*ptr & 0xf]);
    }
}

void
serial_early_init(void)
{
    outb(COM1_PORT + 1, 0x01);// Disable all interrupts except `data available`
    outb(COM1_PORT + 3, 0x80);// Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);// Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);//                  (hi byte)
    outb(COM1_PORT + 3, 0x03);// 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);// Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);// IRQs enabled, RTS/DSR set
    print_string("Initialize serial port output channel\n");
}
