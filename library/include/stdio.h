#ifndef _STDIO_H
#define _STDIO_H

#include <stddef.h>
#include <types.h>
#include <stdarg.h>

#define EOF (-1)

#define _IONBF 0
#define _IOLBF 1
#define _IOFBF 2
#define STDIN_FILENO 0x00
#define STDOUT_FILENO 0x01
#define STDERR_FILENO 0x02

typedef struct {int fd;} FILE;

extern FILE *stdin;
extern FILE *stdout;
extern FILE *stderr;

int fgetc(FILE *stream);
int getchar(void);

int fputc(int c, FILE *stream);
int putchar(int c);

char *fgets(char *str, int n, FILE *stream);

int fputs(const char *str, FILE *stream);
int puts(const char *str);

size_t fread(void *ptr, size_t size, size_t count, FILE *stream);

size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream);

int fflush(FILE *stream);

int printf(const char *format, ...);
int fprintf(FILE *stream, const char *format, ...);
int vprintf(const char *format, va_list ap);

#endif