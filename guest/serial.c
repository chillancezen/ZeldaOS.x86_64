/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <serial.h>

#define COM1_PORT 0x3f8
int
is_transmit_empty(void)
{
    return inb(COM1_PORT + 5) & 0x20;
}

void
write_serial(uint8_t a)
{
    while (is_transmit_empty() == 0);
    outb(COM1_PORT,a);
}


__attribute__((constructor)) void
serial_init(void)
{
    outb(COM1_PORT + 1, 0x01);// Disable all interrupts except `data available`
    outb(COM1_PORT + 3, 0x80);// Enable DLAB (set baud rate divisor)
    outb(COM1_PORT + 0, 0x03);// Set divisor to 3 (lo byte) 38400 baud
    outb(COM1_PORT + 1, 0x00);//                  (hi byte)
    outb(COM1_PORT + 3, 0x03);// 8 bits, no parity, one stop bit
    outb(COM1_PORT + 2, 0xC7);// Enable FIFO, clear them, with 14-byte threshold
    outb(COM1_PORT + 4, 0x0B);// IRQs enabled, RTS/DSR set
}

