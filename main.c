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
#include "fio.h"

int run;

void sighandler(int signal)
{
	run = 0;
}

int main(int argc, char **argv)
{
	FILE *fp;
	irc_t irc;

	fp = fopen("log.txt", "a");

	fio_setfp(fp);
	run = 1;

	if (irc_connect(&irc, "irc.freenode.org", "6667") < 0) {
		fprintf(stderr, "Connection failed.\n");
		goto exit_err;
	}

	if (irc_login(&irc, "brimonk_testbot") < 0) {
		fprintf(stderr, "Couldn't log in.\n");
		goto exit_err;
	}

	if (irc_join_channel(&irc, "#testingbot") < 0) {
		fprintf(stderr, "Couldn't join channel.\n");
		goto exit_err;
	}

	while (irc_handle_data(&irc) >= 0 && run);

	/* print quitting message */

	irc_close(&irc);
	fio_closefp();

	return 0;

exit_err:
	irc_close(&irc);
	fio_closefp();
	return 1;
}

