#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

_Noreturn void _Exit(int status)
{
	_exit(status);
}

_Noreturn void exit(int status)
{
	_exit(status);
}

static int digit_value(int c)
{
    if (c >= '0' && c <= '9')
        return c - '0';

    if (c >= 'a' && c <= 'z')
        return c - 'a' + 10;

    if (c >= 'A' && c <= 'Z')
        return c - 'A' + 10;

    return -1;
}


long strtol(
    const char *str,
    char **endptr,
    int base
)
{
    const char *p = str;

    /*
     * Skip leading whitespace.
     */
    while (isspace((unsigned char)*p))
        ++p;

    /*
     * Sign.
     */
    int negative = 0;

    if (*p == '-') {
        negative = 1;
        ++p;
    } else if (*p == '+') {
        ++p;
    }

    /*
     * Validate base.
     */
    if (base != 0 &&
        (base < 2 || base > 36))
    {
        if (endptr != NULL)
            *endptr = (char *)str;

        errno = EINVAL;

        return 0;
    }

    /*
     * Determine base automatically.
     */
    if (base == 0) {
        if (*p == '0') {
            if (p[1] == 'x' || p[1] == 'X') {
                base = 16;
                p += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        /*
         * strtol("0xff", NULL, 16) is valid.
         */
        if (p[0] == '0' &&
            (p[1] == 'x' || p[1] == 'X'))
        {
            p += 2;
        }
    }

    const char *digits_start = p;

    unsigned long value = 0;

    /*
     * Overflow limits.
     *
     * We accumulate using unsigned long so that we can
     * safely handle LONG_MIN.
     */
    unsigned long limit;

    if (negative)
        limit = (unsigned long)LONG_MAX + 1UL;
    else
        limit = (unsigned long)LONG_MAX;

    while (*p != '\0') {
        int digit = digit_value(
            (unsigned char)*p
        );

        if (digit < 0 || digit >= base)
            break;

        /*
         * Check for overflow before:
         *
         * value = value * base + digit
         */
        if (value > limit / (unsigned long)base ||
            (value == limit / (unsigned long)base &&
             (unsigned long)digit >
             limit % (unsigned long)base))
        {
            errno = ERANGE;

            /*
             * Consume remaining valid digits.
             */
            do {
                ++p;

                digit = digit_value(
                    (unsigned char)*p
                );

            } while (
                digit >= 0 &&
                digit < base
            );

            if (endptr != NULL)
                *endptr = (char *)p;

            if (negative)
                return LONG_MIN;

            return LONG_MAX;
        }

        value = value * (unsigned long)base +
                (unsigned long)digit;

        ++p;
    }

    /*
     * No digits were consumed.
     */
    if (p == digits_start) {
        if (endptr != NULL)
            *endptr = (char *)str;

        return 0;
    }

    if (endptr != NULL)
        *endptr = (char *)p;

    if (negative) {
        if (value == (unsigned long)LONG_MAX + 1UL)
            return LONG_MIN;

        return -(long)value;
    }

    return (long)value;
}

unsigned long strtoul(
    const char *str,
    char **endptr,
    int base
)
{
    const char *p = str;

    while (isspace((unsigned char)*p))
        ++p;

    int negative = 0;

    if (*p == '-') {
        negative = 1;
        ++p;
    } else if (*p == '+') {
        ++p;
    }

    if (base != 0 &&
        (base < 2 || base > 36))
    {
        if (endptr != NULL)
            *endptr = (char *)str;

        errno = EINVAL;

        return 0;
    }

    if (base == 0) {
        if (*p == '0') {
            if (p[1] == 'x' || p[1] == 'X') {
                base = 16;
                p += 2;
            } else {
                base = 8;
            }
        } else {
            base = 10;
        }
    } else if (base == 16) {
        if (p[0] == '0' &&
            (p[1] == 'x' || p[1] == 'X'))
        {
            p += 2;
        }
    }

    const char *digits_start = p;

    unsigned long value = 0;

    while (*p != '\0') {
        int digit = digit_value(
            (unsigned char)*p
        );

        if (digit < 0 || digit >= base)
            break;

        if (value >
            (ULONG_MAX - (unsigned long)digit) /
            (unsigned long)base)
        {
            errno = ERANGE;

            do {
                ++p;

                digit = digit_value(
                    (unsigned char)*p
                );

            } while (
                digit >= 0 &&
                digit < base
            );

            if (endptr != NULL)
                *endptr = (char *)p;

            return ULONG_MAX;
        }

        value = value * (unsigned long)base +
                (unsigned long)digit;

        ++p;
    }

    if (p == digits_start) {
        if (endptr != NULL)
            *endptr = (char *)str;

        return 0;
    }

    if (endptr != NULL)
        *endptr = (char *)p;

    if (negative)
        return 0UL - value;

    return value;
}

int atoi(const char *str)
{
	return (int)strtol(str, NULL, 10);
}


long atol(const char *str)
{
	return strtol(str, NULL, 10);
}
