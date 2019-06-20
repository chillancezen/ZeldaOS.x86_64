/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _SERIAL_H
#define _SERIAL_H
#include <portio.h>

int
is_transmit_empty(void);

void
write_serial(uint8_t a);
#endif
