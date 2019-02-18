/*
 * Copyright (c) 2019 Jie Zheng
 *
 * The IO port Hardware abstraction layer for x86_64
 */
#if defined(CODE32_CONTEXT)
    #include <code32/include/ioport.h>
#else
    #include <x86_64/include/ioport.h>
#endif


