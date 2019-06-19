/*
 * Copyright (c) 2019 Jie Zheng
 */

#include <lib64/include/string.h>


void
memcpy(void * dst, const void * src, int length)
{
    int idx = 0;
    for (idx = 0; idx < length; idx++) {
        *(idx + (char *)dst) = *(idx + (char *)src);
    }
}
void
memset(void * dst, uint8_t val, uint64_t size)
{
    uint64_t idx = 0;
    uint8_t * ptr = (uint8_t *)dst;
    for (; idx < size; idx++) {
        ptr[idx] = val;
    }
}

void
sprintf(char * buff, const char * fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    sprintf_raw(buff, fmt, args);
    va_end(args);
}
#define DEFAULT_RESOLVE_STACK 128

static void
resolve_decimal(char * buff, int * piptr, uint64_t qword)
{
    uint8_t stack[DEFAULT_RESOLVE_STACK];
    int iptr = 0;
    uint8_t mod = 0x0;
    if (qword < 0) {
        return;
    }
    while (qword && iptr < DEFAULT_RESOLVE_STACK) {
        mod = qword % 10;
        stack[iptr++] = '0' + mod;
        qword /= 10;
    }
    if (!iptr) {
        stack[iptr++] = '0';
    }
    while (iptr > 0) {
        buff[*piptr] = stack[--iptr];
        *piptr += 1;
    }
}

static void
resolve_hexadecimal(char * buff, int * piptr, uint64_t qword,
                    uint8_t is_lowercase)
{
   uint8_t stack[DEFAULT_RESOLVE_STACK];
   uint8_t lower[] = "0123456789abcdef";
   uint8_t upper[] = "0123456789ABCDEF";
   int iptr = 0;
   int mod;
   while (qword && iptr < DEFAULT_RESOLVE_STACK) {
       mod = qword & 0xf;
       stack[iptr++] = is_lowercase ? lower[mod] : upper[mod];
       qword = qword >> 4;
   }

   if (!iptr) {
       stack[iptr++] = '0';
   }
   while (iptr > 0) {
       buff[*piptr] = stack[--iptr];
       *piptr += 1;
   }
}
void
sprintf_raw(char * buff, const char * fmt, va_list arg_ptr)
{
    const char * ptr = fmt;
    int iptr = 0;
    for (; *ptr; ptr++) {
        if (*ptr != '%') {
            buff[iptr++] = *ptr;
        } else {
            ptr++;
            switch (*ptr)
            {
                case 's':
                    {
                        char * string_ptr = va_arg(arg_ptr, char *);
                        for (; *string_ptr; string_ptr++) {
                            buff[iptr++] = *string_ptr;
                        }
                    }
                    break;
                case 'c':
                    {
                        char char_arg = (char)va_arg(arg_ptr, uint64_t);
                        buff[iptr++] = char_arg;
                    }
                    break;
                case 'd':
                    {
                        int32_t dword_arg = va_arg(arg_ptr, uint64_t);
                        if (dword_arg < 0) {
                            buff[iptr++] = '-';
                            dword_arg = -dword_arg;
                        }
                        resolve_decimal(buff, &iptr, dword_arg);
                    }
                    break;
                case 'q':
                    {
                        int64_t qword_arg = va_arg(arg_ptr, uint64_t);
                        if (qword_arg < 0) {
                            buff[iptr++] = '-';
                            qword_arg = -qword_arg;
                        }
                        resolve_decimal(buff, &iptr, qword_arg);
                    }
                    break;
                case 'x':
                case 'X':
                    {
                        uint64_t qword_arg = va_arg(arg_ptr, uint64_t);
                        resolve_hexadecimal(buff, &iptr, qword_arg,
                                            *ptr == 'x' ? 1 : 0);
                    }
                    break;
                default:
                    break;
            }
        }
    }
    buff[iptr] = 0x0;
}
