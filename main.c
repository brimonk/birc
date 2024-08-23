// Brian Chrzanowski
// 2024-08-01 01:25:36
//
// TODO
// 1. everyone in the IRC channel gets to play/join
// 2. allow players to work
// 3. use '/names' to fetch all of the channel members and create entries for them

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

i32 main_connect(irc_t *irc, char *address, char *port, i32 use_tls)
{
	// TODO handle use_tls at some point
	memset(irc, 0, sizeof(*irc));
	i32 rc = irc_connect(irc, address, port);
	if (rc < 0) {
		ERR("Connection to '%s:%s' failed! ERR %d", address, port, rc);
	}
	return rc;
}

int main(int argc, char **argv)
{
	i32 rc;
	irc_t irc;

	srand(time(NULL));

	run = 1;

	// rc = main_connect(&irc, "irc.freenode.org", "6667", false);
	// rc = main_connect(&irc, "127.0.0.1", "6667", false);
	rc = main_connect(&irc, "irc.badcop.live", "6667", false);
	if (rc < 0) exit(1);

	if (irc_login(&irc, "rpgman") < 0) {
		ERR("Could not log in as 'rpgman'");
		goto exit_err;
	}

	// if (irc_join_channel(&irc, "#testing") < 0) {
	if (irc_join_channel(&irc, "#test") < 0) {
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

