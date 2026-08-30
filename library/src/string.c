#include <string.h>

size_t strlen(const char *str)
{
	const char *p = str;
	
	while (*p)
		++p;

	return (size_t)(p - str);
}


size_t strnlen(const char *str, size_t maxlen)
{
	size_t i = 0;
	
	while (i < maxlen && str[i])
		++i;

	return i;
}


void *memset(void *dest, int value, size_t n)
{
	unsigned char *p = dest;
	
	while (n--)
		*p++ = (unsigned char)value;
	
	return dest;
}


void *memcpy(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	
	while (n--)
		*d++ = *s++;
	
	return dest;
}


void *memmove(void *dest, const void *src, size_t n)
{
	unsigned char *d = dest;
	const unsigned char *s = src;
	
	if (d < s) {
		while (n--)
			*d++ = *s++;
	} else if (d > s) {
		d += n;
		s += n;
		
		while (n--)
			*--d = *--s;
	}
	
	return dest;
}


int memcmp(const void *a, const void *b, size_t n)
{
	const unsigned char *x = a;
	const unsigned char *y = b;
	
	while (n--) {
		if (*x != *y)
			return (int)*x - (int)*y;
		
		++x;
		++y;
	}
	
	return 0;
}


int strcmp(const char *a, const char *b)
{
	while (*a && *a == *b) {
		++a;
		++b;
	}
	
	return (unsigned char)*a - (unsigned char)*b;
}


int strncmp(const char *a, const char *b, size_t n)
{
	while (n && *a && *a == *b) {
		++a;
		++b;
		--n;
	}
	
	if (!n)
		return 0;
	
	return (unsigned char)*a - (unsigned char)*b;
}


char *strcpy(char *dest, const char *src)
{
	char *result = dest;
	
	while ((*dest++ = *src++));
	
	return result;
}


char *strncpy(char *dest, const char *src, size_t n)
{
	char *result = dest;
	
	while (n && *src) {
		*dest++ = *src++;
		--n;
	}
	
	while (n--)
		*dest++ = '\0';
	
	return result;
}


char *strchr(const char *str, int c)
{
	while (*str) {
		if (*str == (char)c)
			return (char *)str;
		
		++str;
	}
	
	return c == '\0' ? (char *)str : NULL;
}


char *strrchr(const char *str, int c)
{
	const char *last = NULL;
	
	do {
		if (*str == (char)c)
			last = str;
	} while (*str++);
	
	return (char *)last;
}