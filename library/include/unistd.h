#ifndef _UNISTD_H
#define _UNISTD_H

#include <stddef.h>
#include <stdint.h>
#include <types.h>

#define STDIN_FILENO   0
#define STDOUT_FILENO  1
#define STDERR_FILENO  2

ssize_t read(int fd, void *buffer, size_t count);
ssize_t write(int fd, const void *buffer, size_t count);

_Noreturn void _exit(int status);

unsigned int sleep(unsigned int seconds);
int usleep(unsigned int usec);

uint32_t sys_time_ms(void);

#endif