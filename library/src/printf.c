#include <stdio.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

static int put_string(FILE *stream, const char *str)
{
	int count = 0;
	
	if (str == NULL)
		str = "(null)";
	
	while (*str != '\0') {
		if (fputc((unsigned char)*str, stream) == EOF)
			return -1;
		
		++str;
		++count;
	}
	
	return count;
}


static int put_unsigned(FILE *stream, unsigned long value, unsigned int base, int uppercase)
{
	char buffer[32];
	int length = 0;
	int count = 0;
	
	const char *digits;
	
	if (uppercase)
		digits = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	else
		digits = "0123456789abcdefghijklmnopqrstuvwxyz";
	
	/*
	 * Convert backwards into the buffer.
	 */
	do {
		buffer[length++] = digits[value % base];
		value /= base;
	} while (value != 0);
	
	/*
	 * Print forwards.
	 */
	while (length > 0) {
		if (fputc(buffer[--length], stream) == EOF)
			return -1;
		
		++count;
	}
	
	return count;
}


static int put_signed(FILE *stream, long value)
{
	unsigned long magnitude;
	
	if (value < 0) {
		if (fputc('-', stream) == EOF)
			return -1;
		
		/*
		 * Avoid overflow when value == LONG_MIN.
		 */
		magnitude = 0UL - (unsigned long)value;
		
		int result = put_unsigned(
			stream,
			magnitude,
			10,
			0
		);
		
		if (result < 0)
			return -1;
		
		return result + 1;
	}
	
	return put_unsigned(
		stream,
		(unsigned long)value,
		10,
		0
	);
}


static int put_pointer(FILE *stream, const void *pointer)
{
	if (put_string(stream, "0x") < 0)
		return -1;
	
	return put_unsigned(
		stream,
		(uintptr_t)pointer,
		16,
		0
	) + 2;
}


int vfprintf(FILE *stream, const char *format, va_list ap)
{
	int count = 0;
	
	while (*format != '\0') {
		
		/*
		 * Normal character.
		 */
		if (*format != '%') {
			if (fputc((unsigned char)*format, stream) == EOF)
				return -1;
			
			++format;
			++count;
			continue;
		}
		
		++format;
		
		/*
		 * End of string after '%'.
		 */
		if (*format == '\0') {
			if (fputc('%', stream) == EOF)
				return -1;
			
			++count;
			break;
		}
		
		switch (*format) {
			case '%':
				if (fputc('%', stream) == EOF)
					return -1;
				
				count++;
				break;
			
			
			case 'c': {
				int value = va_arg(ap, int);
				
				if (fputc(value, stream) == EOF)
					return -1;
				
				++count;
				break;
			}
			
			
			case 's': {
				const char *value = va_arg(ap, const char *);
				
				int result = put_string(stream, value);
				
				if (result < 0)
					return -1;
				
				count += result;
				break;
			}
			
			
			case 'd':
			case 'i': {
				int value = va_arg(ap, int);
				
				int result = put_signed(stream, (long)value);
				
				if (result < 0)
					return -1;
				
				count += result;
				break;
			}
			
			
			case 'u': {
				unsigned int value = va_arg(ap, unsigned int);
				
				int result = put_unsigned(stream, (unsigned long)value, 10, 0);
				
				if (result < 0)
					return -1;
				
				count += result;
				break;
			}
			
			
			case 'x': {
				unsigned int value = va_arg(ap, unsigned int);
				
				int result = put_unsigned(stream, (unsigned long)value, 16, 0);
				
				if (result < 0)
					return -1;
				
				count += result;
				break;
			}
			
			
			case 'X': {
				unsigned int value = va_arg(ap, unsigned int);
				
				int result = put_unsigned(stream, (unsigned long)value, 16, 1);
				
				if (result < 0)
					return -1;
				
				count += result;
				break;
			}
			
			
			case 'p': {
				const void *value = va_arg(ap, const void *);
				
				int result = put_pointer(stream, value);
				
				if (result < 0)
					return -1;
				
				count += result;
				break;
			}
			
			
			default:
				if (fputc('%', stream) == EOF)
					return -1;
				
				if (fputc((unsigned char)*format, stream) == EOF)
					return -1;
				
				count += 2;
				break;
		}
		
		++format;
	}
	
	return count;
}


int fprintf(FILE *stream, const char *format, ...)
{
	va_list ap;
	
	va_start(ap, format);
	
	int result = vfprintf(stream, format, ap);
	
	va_end(ap);
	
	return result;
}


int printf(const char *format, ...)
{
	va_list ap;
	
	va_start(ap, format);
	
	int result = vfprintf(stdout, format, ap);
	
	va_end(ap);
	
	return result;
}

int vprintf(const char *format, va_list ap)
{
	return vfprintf(
		stdout,
		format,
		ap
	);
}
