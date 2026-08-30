#include <time.h>
#include <unistd.h>
#include <errno.h>

uint32_t time_ms(void)
{
	return sys_time_ms();
}


time_t time(time_t *timer)
{
	time_t result = (time_t)(sys_time_ms() / 1000U);
	
	if (timer)
		*timer = result;
	
	return result;
}


int clock_gettime(int clock_id, struct timespec *tp)
{
	if (!tp) {
		errno = EFAULT;
		return -1;
	}
	
	if (clock_id != CLOCK_REALTIME &&
		clock_id != CLOCK_MONOTONIC) {
		errno = EINVAL;
		return -1;
	}
	
	uint32_t milliseconds = sys_time_ms();
	
	tp->tv_sec = milliseconds / 1000U;
	tp->tv_nsec = (long)(milliseconds % 1000U) * 1000000L;
	
	return 0;
}