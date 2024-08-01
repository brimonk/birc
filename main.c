// Brian Chrzanowski
// 2024-08-01 01:25:36
//
// An IRC RPG bot for @badcop_'s IRC system. Please send me a message (somehow) to have this bot
// in other servers :).
//
// TODO
// 1. New Characters

/*
 * Brian Chrzanowski
 * Wed Feb 20, 2019 03:04
 *
 * Brian Chrzanowski's IRC Bot
 *
 * TODO (Brian)
 * Static C Source Inclusion, for programmable modules
 */

#define COMMON_IMPLEMENTATION
#include "common.h"

#include "irc.h"

int run;

void sighandler(int signal)
{
	run = 0;
}

int main(int argc, char **argv)
{
	irc_t irc;

	srand(time(NULL));

	run = 1;

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

