/*
 * Copyright (c) 2019 Jie Zheng
 *
 *  This is the Zelda64 Config File for both bootloader and kernel64
 */

// Note the AP boot address must be lower than 0xVV000 whether VV is the SIPI
// vector number. it must be 4K aligned
#define AP_BOOT_BASE 0x11000
