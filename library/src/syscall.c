#include <stdint.h>
#include <stddef.h>

#include <errno.h>
#include <unistd.h>
#include <syscall.h>

static int syscall_errno(int32_t result)
{
	switch (-result) {
		case SYSCALL_EACCES:
			return EACCES;
        case SYSCALL_EFAULT:
			return EFAULT;
		default:
			return EINVAL;
	}
}


ssize_t read(int fd, void *buffer, size_t count)
{
	syscall_result_t r;
	
	r = syscall(0x84, SYS_READ, (uint32_t)fd, (uint32_t)buffer, (uint32_t)count);
	
	int32_t result = (int32_t)r.eax;
	
	if (result < 0) {
		errno = syscall_errno(result);
		return -1;
	}
	
	return result;
}


ssize_t write(int fd, const void *buffer, size_t count)
{
	syscall_result_t r;
	
	r = syscall(0x84, SYS_WRITE, (uint32_t)fd, (uint32_t)buffer, (uint32_t)count);
	
	int32_t result = (int32_t)r.eax;
	
	if (result < 0) {
		errno = syscall_errno(result);
		return -1;
	}
	
	return result;
}


_Noreturn void _exit(int status)
{
	syscall(0x84, SYS_EXIT, (uint32_t)status, 0x00, 0x00);
	
	for (;;) {
		asm volatile ("ud2");
	}
}


uint32_t sys_time_ms(void)
{
	syscall_result_t r;
	
	r = syscall(0x84, SYS_TIME_MS, 0, 0, 0);
	
	return r.eax;
}


unsigned int sleep(unsigned int seconds)
{
	syscall(0x84, SYS_SLEEP, seconds * 1000U, 0, 0);
	
	return 0;
}


int usleep(unsigned int usec)
{
	unsigned int milliseconds = (usec + 999U) / 1000U;
	
	syscall(0x84, SYS_SLEEP, milliseconds, 0, 0);
	
	return 0;
}
