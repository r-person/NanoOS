#ifndef _SYS_SYSCALL_H
#define _SYS_SYSCALL_H

#include <stdint.h>

typedef struct syscall_result_t {
    uint32_t eax;
    uint32_t ebx;
    uint32_t ecx;
} syscall_result_t;

#define SYS_GET_VERSION  0x00
#define SYS_WRITE        0x01
#define SYS_EXIT         0x02
#define SYS_SLEEP        0x03
#define SYS_TIME_MS      0x04
#define SYS_READ         0x05

#define SYSCALL_EACCES   0x01
#define SYSCALL_EFAULT   0x02

#define syscall(n, a, b, c, d)					\
({												\
    syscall_result_t r;							\
    uint32_t _eax = (a);						\
    uint32_t _ebx = (b);						\
    uint32_t _ecx = (c);						\
    uint32_t _edx = (d);						\
												\
    asm volatile (								\
        "int $" #n								\
        : "+a"(_eax), "+b"(_ebx), "+c"(_ecx)	\
        : "d"(_edx)								\
        : "memory"								\
    );											\
												\
    r.eax = _eax;								\
    r.ebx = _ebx;								\
    r.ecx = _ecx;								\
    r;											\
})

#endif