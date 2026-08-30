#ifndef _STDLIB_H
#define _STDLIB_H

#include <stddef.h>

_Noreturn void exit(int status);
_Noreturn void _Exit(int status);

int atoi(const char *str);
long atol(const char *str);

long strtol(
    const char *str,
    char **endptr,
    int base
);

unsigned long strtoul(
    const char *str,
    char **endptr,
    int base
);

#endif