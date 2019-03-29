/*
 * Copyright (c) 2019 Jie Zheng
 *
 *  This is the Zelda64 Config File for both bootloader and kernel64
 */

#define DEFAULT_LOGGING_LEVEL LOG_DEBUG

// Note the AP boot address must be lower than 0xVV000 whether VV is the SIPI
// vector number. it must be 4K aligned

#define AP_BOOT_BASE 0x11000

#define EARLY_PML4_BASE 0x12000
#define EARLY_PDPT_BASE 0x13000
#define EARLY_PD_BASE   0x14000
