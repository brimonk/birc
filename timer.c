#include "common.h"

#include <pthread.h>
#include <sys/time.h>
#include <unistd.h>

// Brian Chrzanowski
//
// This is a small timer library. This will allow us to enqueue functions to execute at the given
// timeval in the future. Whenever a new function is enqueued, the 'g_timer_functions' array is
// sorted. This is locked behind a mutex.

#define TIMER_SLEEP_TIMEOUT_USEC (1000 * 1000)

typedef struct TimerFunction {
	struct timeval tv;
	void *(*fn)(void *);
	void *context;
	int executed;
} TimerFunction;

static int timer_run = true;
static TimerFunction *g_timer_functions = NULL;
static pthread_t g_timer_thread = {0};
static pthread_mutex_t g_timer_functions_mutex = {0};

int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y);

int timer_function_comp(const void *a, const void *b)
{
	TimerFunction *tfna = (TimerFunction *)a;
	TimerFunction *tfnb = (TimerFunction *)b;

	struct timeval result;

	int rc = timeval_subtract(&result, &tfna->tv, &tfnb->tv);
	return rc == 1 ? -1 : 1;
}

void *timer_runner(void *ptr)
{
	while (timer_run) {

		struct timeval now = {0}, result = {0};
		gettimeofday(&now, NULL);

		pthread_mutex_lock(&g_timer_functions_mutex);

		for (size_t i = 0; i < arrlen(g_timer_functions); i++) {
			TimerFunction *curr = &g_timer_functions[i];

			LOG("%ld - %ld.%ld", i, curr->tv.tv_sec, curr->tv.tv_usec);

			int rc = timeval_subtract(&result, &curr->tv, &now);
			if (rc == 1) { // negative, this timer has elapsed, run the function
				curr->fn(curr->context);
				curr->executed = true;
			}
		}

		// Because we sort these things by timestamp, it should be the case that we can repeatedly
		// pop the first element from the array here, and keep doing it until the executed member
		// is false.

		while (arrlen(g_timer_functions) > 0 && g_timer_functions[0].executed) {
			arrdel(g_timer_functions, 0);
		}

		pthread_mutex_unlock(&g_timer_functions_mutex);

		usleep(TIMER_SLEEP_TIMEOUT_USEC); // 1 sec by default?
	}

	return NULL;
}

__attribute__((constructor))
void timer_init(void)
{
	LOG("TIMER START");

	timer_run = true;
	pthread_mutex_init(&g_timer_functions_mutex, NULL);
	pthread_create(&g_timer_thread, NULL, timer_runner, NULL);

	LOG("TIMER STARTED");
}

__attribute__((destructor))
void timer_free(void)
{
	LOG("TIMER END");

	timer_run = false;
	pthread_join(g_timer_thread, NULL);
	pthread_mutex_destroy(&g_timer_functions_mutex);

	LOG("TIMER ENDED");
}

// timer_fn_enqueue: enqueues the function with the context to execute at the tv in the future
// pass a 'tv' value that has already elapsed for this function to run on the next timer iter
int timer_fn_enqueue(struct timeval tv, void *(*fn)(void *), void *context)
{
	pthread_mutex_lock(&g_timer_functions_mutex);
	TimerFunction tfn = {
		.tv = tv,
		.fn = fn,
		.context = context,
		.executed = false,
	};
	arrput(g_timer_functions, tfn);
	qsort(g_timer_functions, arrlen(g_timer_functions), sizeof(*g_timer_functions), timer_function_comp);
	pthread_mutex_unlock(&g_timer_functions_mutex);
	return 0;
}

// timer_get_time: returns the current time 'sec' and 'msec' in the future
struct timeval timer_get_time(i32 sec, i32 msec)
{
	struct timeval tv = {0};
	gettimeofday(&tv, NULL);
	tv.tv_sec += sec;
	tv.tv_usec += msec * 1000;
	return tv;
}

// Subtract values 'y' from 'x', storing the result in 'result'. Return 1 if the diff is negative.
int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y)
{
	struct timeval tvx = *x;
	struct timeval tvy = *y;

	// perform the carry for the later subtraction by updating y
	if (tvx.tv_usec < tvy.tv_usec) {
		int nsec = (tvy.tv_usec - tvx.tv_usec) / 1000000 + 1;
		tvy.tv_usec -= 1000000 * nsec;
		tvy.tv_sec += nsec;
	}

	if (tvx.tv_usec - tvy.tv_usec > 1000000) {
		int nsec = (tvx.tv_usec - tvy.tv_usec) / 1000000;
		tvy.tv_usec += 1000000 * nsec;
		tvy.tv_sec -= nsec;
	}

	// compute the time remaining to wait - tv_usec is certainly positive
	result->tv_sec = tvx.tv_sec - tvy.tv_sec;
	result->tv_usec = tvx.tv_usec - tvy.tv_usec;

	// return 1 if result is negative
	return tvx.tv_sec < tvy.tv_sec;
}
