#ifndef TIMER_H
#define TIMER_H

#include <sys/time.h>

// timer_fn_enqueue: enqueues the function with the context to execute at the tv in the future
// pass a 'tv' value that has already elapsed for this function to run on the next timer iter
int timer_fn_enqueue(struct timeval tv, void *(*fn)(void *), void *context);

#endif // TIMER_H
