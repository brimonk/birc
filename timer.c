#include "common.h"

#include <pthread.h>
#include <sys/time.h>

__attribute__((constructor))
void timer_init(void)
{
	LOG("TIMER START");
}

__attribute__((destructor))
void timer_free(void)
{
	LOG("TIMER END");
}
