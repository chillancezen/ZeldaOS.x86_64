/*
 * Copyright (c) 2019 Jie Zheng
 */
#ifndef _PIT_H
#define _PIT_H
#define PIT_CHANNEL0_PORT 0x40
#define PIT_CHANNEL1_PORT 0x41
#define PIT_CHANNEL2_PORT 0x42
#define PIT_CONTROL_PORT 0x43

#define OSCILLATPR_CHIP_FREQUENCY 1193182
void pit_init(void);

#endif
