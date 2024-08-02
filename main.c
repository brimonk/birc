// Brian Chrzanowski
// 2024-08-01 01:25:36
//
// TODO
// 1. everyone in the IRC channel gets to play/join
// 2. allow players to work

#define COMMON_IMPLEMENTATION
#include "common.h"

#include "irc.h"
#include "timer.h"

#include <sys/time.h>

int run;

void sighandler(int signal)
{
	run = 0;
}

void *timer_function_test(void *arg)
{
	LOG("TIMER FUNCTION TEST WORKED! - %s", (char *)arg);
}

int main(int argc, char **argv)
{
	irc_t irc;

	srand(time(NULL));

	run = 1;

	// TESTING THE TIMER FUNCTIONS

	char *strings[] = {
		"STRING 9",
		"STRING 8",
		"STRING 7",
		"STRING 6",
		"STRING 5",
		"STRING 4",
		"STRING 3",
		"STRING 2",
		"STRING 1",
	};


	struct timeval curr;
	gettimeofday(&curr, NULL);
	for (size_t i = 0; i < ARRSIZE(strings); i++) {
		curr.tv_sec += 1;
		timer_fn_enqueue(curr, timer_function_test, strings[i]);
	}

#if 0
	if (irc_connect(&irc, "irc.freenode.org", "6667") < 0) {
		fprintf(stderr, "Connection failed.\n");
		goto exit_err;
	}
#else
	// Why doesn't 'localhost' work here?
	if (irc_connect(&irc, "127.0.0.1", "6667") < 0) {
		ERR("Connection failed");
		goto exit_err;
	}
#endif

	if (irc_login(&irc, "rpgman") < 0) {
		ERR("Could not log in as 'rpgman'");
		goto exit_err;
	}

	if (irc_join_channel(&irc, "#testing") < 0) {
		ERR("Could not join channel '#testing'");
		goto exit_err;
	}

	while (irc_handle_data(&irc) >= 0 && run);

	/* print quitting message */

	irc_close(&irc);

	return 0;

exit_err:
	irc_close(&irc);
	return 1;
}

