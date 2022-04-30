// Copyright (C) 2010 by Shinichiro Hamaji
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#include <sys/syscall.h>

#if defined(__x86_64__)
#define RAW_WRITE(fd, buf, count)                                                     \
    do {                                                                              \
        register unsigned long RAW_rsi##__LINE__ __asm__("rsi") = (unsigned long)buf; \
        __asm__ volatile("syscall;\n"                                                 \
                         : "+r"(RAW_rsi##__LINE__)                                    \
                         : "a"(SYS_write), "D"(fd), "d"(count)                        \
                         : "r8", "r10", "rcx", "r11", "memory", "cc");                \
        /* The input registers may be broken by syscall */                            \
        __asm__ volatile("" ::: "rax", "rdi", "rdx");                                 \
    } while (0)
#elif defined(__i386__)
#define RAW_WRITE(fd, buf, count)                                                                          \
    do {                                                                                                   \
        __asm__ volatile("int $0x80;\n" ::"a"(SYS_write), "b"(fd), "c"(buf), "d"(count) : "memory", "cc"); \
        /* The input registers may be broken by syscall */                                                 \
        __asm__ volatile("" ::: "eax", "ebx", "ecx", "edx");                                               \
    } while (0)
#elif defined(__arm__)
#define RAW_WRITE(fd, buf, count)                \
    do {                                         \
        __asm__ volatile(                        \
            "mov %%r0, %1;\n"                    \
            "mov %%r1, %2;\n"                    \
            "mov %%r2, %3;\n"                    \
            "push {%%r7};\n"                     \
            "mov %%r7, %0;\n"                    \
            "swi 0x0;\n"                         \
            "pop {%%r7};\n" ::"I"(SYS_write),    \
            "r"(fd), "r"(buf), "r"(count)        \
            : "memory", "cc", "r0", "r1", "r2"); \
    } while (0)
#else
#error "RAW_WRITE isn't defined for this architecture"
#endif

#define RAW_PRINT_STR(buf)                     \
    do {                                       \
        const char* RAW_p##__LINE__ = buf;     \
        int i;                                 \
        for (i = 0; RAW_p##__LINE__[i]; i++) { \
        }                                      \
        RAW_WRITE(2, RAW_p##__LINE__, i);      \
    } while (0)

#define RAW_PRINT_BASE_N(num, base)                    \
    do {                                               \
        /* unsigned long long isn't supported */       \
        long long RAW_n##__LINE__ = (long long)num;    \
        int RAW_b##__LINE__ = base;                    \
        int was_minus = 0;                             \
        char buf[21];                                  \
        char* p = buf + 20;                            \
        int l = 0;                                     \
        if (RAW_n##__LINE__ < 0) {                     \
            was_minus = 1;                             \
            RAW_n##__LINE__ = -RAW_n##__LINE__;        \
        }                                              \
        do {                                           \
            int v = RAW_n##__LINE__ % RAW_b##__LINE__; \
            if (v > 9)                                 \
                *p = 'a' + v - 10;                     \
            else                                       \
                *p = '0' + v;                          \
            l++;                                       \
            RAW_n##__LINE__ /= RAW_b##__LINE__;        \
            p--;                                       \
        } while (RAW_n##__LINE__ != 0);                \
        if (was_minus) {                               \
            *p = '-';                                  \
            l++;                                       \
        } else {                                       \
            p++;                                       \
        }                                              \
        RAW_WRITE(2, p, l);                            \
    } while (0)

#define RAW_PRINT_HEX(num) RAW_PRINT_BASE_N(num, 16)
#define RAW_PRINT_INT(num) RAW_PRINT_BASE_N(num, 10)
#define RAW_PRINT_PTR(num)                        \
    do {                                          \
        char buf[3] = "0x";                       \
        RAW_WRITE(2, buf, 2);                     \
        RAW_PRINT_BASE_N((unsigned long)num, 16); \
    } while (0)

#define RAW_PRINT_NL_AFTER_SOMETHING(print) \
    do {                                    \
        print;                              \
        char buf[2] = "\n";                 \
        RAW_WRITE(2, buf, 1);               \
    } while (0)
#define RAW_PUTS_STR(buf) RAW_PRINT_NL_AFTER_SOMETHING(RAW_PRINT_STR(buf))
#define RAW_PUTS_HEX(buf) RAW_PRINT_NL_AFTER_SOMETHING(RAW_PRINT_HEX(buf))
#define RAW_PUTS_INT(buf) RAW_PRINT_NL_AFTER_SOMETHING(RAW_PRINT_INT(buf))
#define RAW_PUTS_PTR(buf) RAW_PRINT_NL_AFTER_SOMETHING(RAW_PRINT_PTR(buf))

/* some more utilities for "printf" debug... */
#if defined(__arm__)
#define RAW_BREAK() __asm__ volatile("bkpt;\n")
#else
#define RAW_BREAK() __asm__ volatile("int3;\n")
#endif
#define RAW_NOP() __asm__ volatile("nop;\n")
/* you can easily find this code by grepping 4242 */
#if defined(__x86_64__)
#define RAW_UNIQ_NOP() __asm__ volatile("nop 0x42424242(%rax);\n")
#elif defined(__i386__)
#define RAW_UNIQ_NOP() __asm__ volatile("nop 0x42424242(%eax);\n")
#elif defined(__arm__)
#define RAW_UNIQ_NOP() __asm__ volatile("nop;\n")
#else
#error "RAW_UNIQ_NOP isn't defined for this architecture"
#endif
