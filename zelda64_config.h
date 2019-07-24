/*
 * Copyright (c) 2019 Jie Zheng
 *
 *  This is the Zelda64 Config File for both bootloader and kernel64
 */

// log verbosity option[LOG_TRIVIA, LOG_DEBUG, LOG_INFO, LOG_WARN, LOG_ERROR]
#define DEFAULT_LOGGING_LEVEL LOG_DEBUG

// the maximum number of processors supported
#define MAX_NR_CPUS 64
#define STACK_SIZE_PER_CPU (2 * 1024 * 1024)

// The physical memory detected from BIOS functions is stored in this page
#define MEMORY_LIST_BASE 0x8400
#define MEMORY_LIST_LONG_MODE_BASE (0x7c00 + MEMORY_LIST_BASE)

// Note the AP boot address must be lower than 0xVV000 whether VV is the SIPI
// vector number. it must be 4K aligned
#define AP_BOOT_BASE 0x11000

#define EARLY_PML4_BASE 0x12000
#define EARLY_PDPT_BASE 0x13000
#define EARLY_PD_BASE   0x14000


// kernel code base for bsp
#define KERNEL_BSP_BASE 0x100000
#define KERNEL_AP_BASE 0x100200

// The maximum memory capacity in GB, as we are going to use a bitmap to
// maintain the physical page usage, this bitmap is designed static.
#define MAX_MEMORY_CAPACITY_IN_GIGABYTE 64


// The initial virtual address ceiling which is mapped as 2MB pages
#define INITIAL_VA_CEILING 0x80000000


// The flag to debug
#define DEBUG 1

// maximum frames to dump in backtrace
#define MAX_FRAMES_TO_DUMP 32

// I find no feasible way to detect the CPU speed but to define one
// FIXME: PLEASE ALSO SPECIFY ONE FOR THE MACHINE YOU ARE GOING TO RUN
#define CPU_FREQUENCY 2494254000
