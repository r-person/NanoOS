#include <stdio.h>
#include <unistd.h>
#include <string.h>

static FILE __stdin = {
	.fd = STDIN_FILENO
};

static FILE __stdout = {
	.fd = STDOUT_FILENO
};

static FILE __stderr = {
	.fd = STDERR_FILENO
};

FILE *stdin = &__stdin;
FILE *stdout = &__stdout;
FILE *stderr = &__stderr;


int fgetc(FILE *stream)
{
	unsigned char c;
	
	ssize_t result = read(stream->fd, &c, 1);
	
	if (result <= 0)
		return EOF;
	
	return (int)c;
}


int getchar(void)
{
	return fgetc(stdin);
}


int fputc(int c, FILE *stream)
{
	unsigned char ch = (unsigned char)c;
	
	if (write(stream->fd, &ch, 1) != 1)
		return EOF;
	
	return (int)ch;
}


int putchar(int c)
{
	return fputc(c, stdout);
}


int fputs(const char *str, FILE *stream)
{
	size_t length = strlen(str);
	
	if (write(stream->fd, str, length) < 0)
		return EOF;
	
	return 0;
}


int puts(const char *str)
{
	if (fputs(str, stdout) == EOF)
		return EOF;
	
	if (putchar('\n') == EOF)
		return EOF;
	
	return 0;
}


char *fgets(char *str, int n, FILE *stream)
{
	if (n <= 0)
		return NULL;
	
	int i = 0;
	
	while (i < n - 1) {
		int c = fgetc(stream);
		
		if (c == EOF) {
			if (i == 0)
				return NULL;
			
			break;
		}
		
		str[i++] = (char)c;
		
		if (c == '\n')
			break;
	}
	
	str[i] = '\0';
	
	return str;
}


size_t fread(void *ptr, size_t size, size_t count, FILE *stream)
{
	if (size == 0 || count == 0)
		return 0;
	
	size_t total = size * count;
	
	ssize_t result = read(stream->fd, ptr, total);
	
	if (result <= 0)
		return 0;
	
	return (size_t)result / size;
}


size_t fwrite(const void *ptr, size_t size, size_t count, FILE *stream)
{
	if (size == 0 || count == 0)
		return 0;
	
	size_t total = size * count;
	
	ssize_t result = write(stream->fd, ptr, total);
	
	if (result <= 0)
		return 0;
	
	return (size_t)result / size;
}


int fflush(FILE *stream)
{
	(void)stream;
	
	return 0;
}