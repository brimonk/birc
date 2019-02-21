/*
 * Brian Chrzanowski
 * Wed Feb 20, 2019 03:07
 *
 * IRC Library Functions
 */

#include <stdio.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <errno.h>

#include "socket.h"
#include "irc.h"
#include "fio.h"
#include "common.h"

/* function declarations */
static int irc_botcmd_help(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_ping(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_smack(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_google(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_insult(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_wiki(irc_t *irc, char *irc_nick, char *arg);

struct ircfunc_t {
	char *command;
	char *usage;
	int (*func)(irc_t *, char *, char *);
};

static struct ircfunc_t ircfuncs[] = {
	{"help",   "USAGE: !help <command>",  irc_botcmd_help},
	{"ping",   "USAGE: !ping",            irc_botcmd_ping},
	{"smack",  "USAGE: !smack <person>",  irc_botcmd_smack},
	{"google", "USAGE: !google <search>", irc_botcmd_google},
	{"insult", "USAGE: !insult <person>", irc_botcmd_insult},
	{"wiki",   "USAGE: !wiki <search>",   irc_botcmd_wiki}
};

/* irc_connect : connect to an irc server */
int irc_connect(irc_t *irc, const char* server, const char* port)
{
	if ((irc->s = get_socket(server, port)) < 0) {
		return -1;
	}

	irc->bufptr = 0;

	/* seed the RNG machine */
	srand(time(NULL));

	return 0;
}

int irc_login(irc_t *irc, const char* nick)
{
	return irc_reg(irc->s, nick, "brimonk", "brimonk test bot");
}

int irc_join_channel(irc_t *irc, const char* channel)
{
	strncpy(irc->channel, channel, 254);
	irc->channel[254] = '\0';
	return irc_join(irc->s, channel);
}

int irc_leave_channel(irc_t *irc)
{
	return irc_part(irc->s, irc->channel);
}

int irc_handle_data(irc_t *irc)
{
	char tempbuffer[512];
	int rc, i;

	/* wait for and receive data from the server */
	if ((rc = sck_recv(irc->s, tempbuffer, sizeof(tempbuffer) - 2)) <= 0) {
		FIO_PRINTF(FIO_ERR, "Got -1 From Socket %s", strerror(errno));
		return -1;
	}

	tempbuffer[rc] = '\0';

	for (i = 0; i < rc; i++) {
		switch (tempbuffer[i]) {
		case '\r':
		case '\n':
			irc->servbuf[irc->bufptr] = '\0';
			irc->bufptr = 0;

			if (strlen(irc->servbuf) == 0) {
				return 0;
			}

			FIO_PRINTF(FIO_LOG, "%s", irc->servbuf);

			if (irc_parse_action(irc) < 0)
				return -1;

			break;

		default:
			irc->servbuf[irc->bufptr] = tempbuffer[i];
			if (irc->bufptr >= (sizeof(irc->servbuf) -1))
				; // Overflow!
			else
				irc->bufptr++;
		}
	}

	return 0;
}

/* irc_parse_action : parses the incoming action the server's sending us */
int irc_parse_action(irc_t *irc)
{
	char *ptr;
	int privmsg;
	char irc_nick[128];
	char irc_msg[512];

	privmsg = 0;

	if (strncmp(irc->servbuf, "PING :", 6) == 0) { /* see if it's a ping */
		return irc_pong(irc->s, &irc->servbuf[6]);

	} else if (strncmp(irc->servbuf, "NOTICE AUTH :", 13) == 0) {
		/* we really don't care about NOTICE AUTH junk */
		return 0;

	} else if (strncmp(irc->servbuf, "ERROR :", 7) == 0) {
		/* log the fact that the server sent us an error and move on */
		return 0;

	} else {
		/* parse the message to get nick, channel, message */

		*irc_nick = '\0';
		*irc_msg = '\0';

		/* see if we have a non-message string */
		if (strchr(irc->servbuf, 1) != NULL)
			return 0;

		if (irc->servbuf[0] == ':') {
			ptr = strtok(irc->servbuf, "!");

			if (ptr == NULL) {
				printf("ptr == NULL\n");
				return 0;
			} else {
				strncpy(irc_nick, &ptr[1], 127);
				irc_nick[127] = '\0';
			}

			while ((ptr = strtok(NULL, " ")) != NULL) {
				if (strcmp(ptr, "PRIVMSG") == 0) {
					privmsg = 1;
					break;
				}
			}

			if (privmsg) {
				if ((ptr = strtok(NULL, ":")) != NULL &&
						(ptr = strtok(NULL, "")) != NULL) {
					strncpy(irc_msg, ptr, 511);
					irc_msg[511] = '\0';
				}
			}

			if (privmsg && strlen(irc_nick) > 0 && strlen(irc_msg) > 0) {
				irc_log_message(irc, irc_nick, irc_msg);
				if (irc_reply_message(irc, irc_nick, irc_msg) < 0)
					return -1;
			}
		}
	}

	return 0;
}

/* irc_reply_message : checks if someone calls on the bot */
int irc_reply_message(irc_t *irc, char *irc_nick, char *msg)
{
	char *command;
	char *arg;
	int i;

	if (*msg != '!')
		return 0;

	/* get the actual command */
	command = strtok(&msg[1], " ");
	arg = strtok(NULL, "");

	if (arg != NULL) {
		while (*arg == ' ')
			arg++;
	}

	if (command == NULL)
		return 0;

	/* spin through the table we defined above */
	for (i = 0; i < ARRSIZE(ircfuncs); i++) {
		if (strcmp(command, ircfuncs[i].command) == 0) {
			return ircfuncs[i].func(irc, irc_nick, arg);
		}
	}

#if 0
	if (strcmp(command, "ping") == 0) {
		return irc_botcmd_ping(irc, irc_nick, arg);
	}

	if (strcmp(command, "smack") == 0) {
		return irc_botcmd_smack(irc, irc_nick, arg);
	}

	if (strcmp(command, "google") == 0) {
		return irc_botcmd_google(irc, irc_nick, arg);
	}

	if (strcmp(command, "insult") == 0) {
		return irc_botcmd_insult(irc, irc_nick, arg);
	}

	if (strcmp(command, "wiki") == 0) {
		return irc_botcmd_wiki(irc, irc_nick, arg);
	}
#endif

	return 0;
}

/* irc_botcmd_help : handles help command and prints command usage info */
static int irc_botcmd_help(irc_t *irc, char *irc_nick, char *arg)
{
	int i, len;
	char buf[256];

	/*
	 * if no arguments are present, we print all of the available commands
	 * if arguments were passed, check if it's a command, if so, print the usage
	 */

	if (arg) {
	} else {
		snprintf(buf, sizeof(buf), "Commands: ");
		for (i = 0, len = strlen(buf); i < ARRSIZE(ircfuncs) || len >= 200;
				i++, len = strlen(buf)) {
			snprintf(buf + len, sizeof(buf) - len, "!%s ", ircfuncs[i].command);
		}
	}

	irc_msg(irc->s, irc->channel, buf);

	return 0;
}

/* irc_botcmd_wiki : adds a sloo of wiki functionality */
static int irc_botcmd_wiki(irc_t *irc, char *irc_nick, char *arg)
{
	return 0;
}

/* irc_botcmd_insult : picks insult from a file and throws shade */
static int irc_botcmd_insult(irc_t *irc, char *irc_nick, char *arg)
{
	/*
	 * TODO (Brian)
	 * As of writing, I'm not super certain what the best way to do this is.
	 * Ideally, all of the insults are stored in a file; however, if the file
	 * isn't there, what happens? Does it fail silently and pop an error in the
	 * log? Does it tell people in IRC the error?
	 *
	 * As of now, we'll have a statically defined file (path, data/insults.txt)
	 * and if we can't open the file, we'll throw an error in the log and
	 * silently fail in IRC.
	 *
	 * It's also a little "slow" because we don't keep the results in memory, we
	 * just read it from a file whenever we want.
	 */

	FILE *insultfp;
	char *ptr;
	char *args[2];
	int lines, randline, i;
	char mesg[512], insultbuf[256];

	memset(args, 0, sizeof(args));
	memset(mesg, 0, sizeof(mesg));
	memset(insultbuf, 0, sizeof(insultbuf));

#define INSULTFILE "data/insults.txt"

	/* try to open the file and get a random line */
	insultfp = fopen(INSULTFILE, "r");
	if (!insultfp) {
		FIO_PRINTF(FIO_ERR, "Couldn't open %s : file not found", INSULTFILE);
		return 0;
	}

	lines = fio_lines(insultfp);
	randline = rand() % lines;

	fio_getline(insultfp, insultbuf, sizeof(insultbuf), randline);

	/* see if we need to remove the newline from the insult */
	if (insultbuf[strlen(insultbuf) - 1] == '\n') {
		insultbuf[strlen(insultbuf) - 1] = '\0';
	}

	return irc_msg(irc->s, irc->channel, insultbuf);

	/* retrieve up to two tokens, one for the */
	ptr = strtok(arg, " ");
	for (i = 0; i < 2 && ptr; i++, ptr = strtok(NULL, " ")) {
		args[i] = ptr;
	}

	if (strcmp(args[0], "insult") != 0) {
		return 0; /* don't want to die just because of bad command */
	}

	if (args[1]) {
	} else {
	}

	fclose(insultfp);

	return 0;
}

/* irc_botcmd_ping : responds to a user with "pong" */
static int irc_botcmd_ping(irc_t *irc, char *irc_nick, char *arg)
{
	if (irc_msg(irc->s, irc->channel, "pong") < 0)
		return -1;

	return 0;
}

/* irc_botcmd_smack : smacks someone over TCP/IP */
static int irc_botcmd_smack(irc_t *irc, char *irc_nick, char *arg)
{
	int damage;
	char mesg[512];

	damage = rand() % 21 + 1;

	if (!arg) { /* if we have an argument, we'll smack the arg */
		arg = irc_nick;
	}

	snprintf(mesg, 511, "smacks %s for %d damage%s.",
			arg, damage, damage == 20 ? " (SUPER EFFECTIVE)" : "");

	mesg[511] = '\0'; /* ensure we have a NULL terminated string */

	if (irc_action(irc->s, irc->channel, mesg) < 0)
		return -1;

	return 0;
}

/* irc_botcmd_google : IRC command for generating Google Links */
static int irc_botcmd_google(irc_t *irc, char *irc_nick, char *arg)
{
	int len;
	char mesg[512];
	char link[256];

	memset(link, 0, 256); /* clean the buffer */

	if (!arg)
		return 0;

	/* encode web safe bytes one at a time */
	for (len = strlen(link);
			len < sizeof(link) - 4 && *arg; arg++, len = strlen(link)) {
		if (*arg < 48) { /* encoding */
			snprintf(link + len, sizeof(link)-len, "%%%02x", *arg);
		} else { /* no encoding */
			snprintf(link + len, sizeof(link)-len, "%c", *arg);
		}
	}

	if (len > sizeof(link) - 4) {
		snprintf(mesg, 511, "search too long for IRC, search it yourself");
	} else {
		snprintf(mesg, 511, "https://www.google.com/search?q=%s", link);
		mesg[511] = '\0';
	}

	if (irc_msg(irc->s, irc->channel, mesg) < 0)
		return -1;

	return 0;
}

int irc_log_message(irc_t *irc, const char* nick, const char* message)
{
	char timestring[128];
	time_t curtime;

	time(&curtime);
	strftime(timestring, 127, "%F - %H:%M:%S", localtime(&curtime));
	timestring[127] = '\0';

	FIO_PRINTF(FIO_LOG, "%s [%s] <%s> %s\n",
			irc->channel, timestring, nick, message);

	return 0;
}

void irc_close(irc_t *irc)
{
	close(irc->s);
}

/* irc_pong : answers pong requests */
int irc_pong(int s, const char *data)
{
	return sck_sendf(s, "PONG :%s\r\n", data);
}

/* irc_reg : registers user upon login */
int irc_reg(int s, const char *nick, const char *username, const char *fullname)
{
	return sck_sendf(s, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

/* irc_join : joins channels */
int irc_join(int s, const char *data)
{
	return sck_sendf(s, "JOIN %s\r\n", data);
}

/* irc_part : sends the PART command to the server */
int irc_part(int s, const char *data)
{
	return sck_sendf(s, "PART %s\r\n", data);
}

/* irc_nick : changes irc nickname */
int irc_nick(int s, const char *data)
{
	return sck_sendf(s, "NICK %s\r\n", data);
}

/* irc_quit : quits irc */
int irc_quit(int s, const char *data)
{
	return sck_sendf(s, "QUIT :%s\r\n", data);
}

/* irc_topic : sets/removes the topic of a channel */
int irc_topic(int s, const char *channel, const char *data)
{
	return sck_sendf(s, "TOPIC %s :%s\r\n", channel, data);
}

/* irc_action : executes an action (.e.g /me is hungry) */
int irc_action(int s, const char *channel, const char *data)
{
	FIO_PRINTF(FIO_LOG, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
	return sck_sendf(s, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
}

/* irc_msg : sends a channel message or a query */
int irc_msg(int s, const char *channel, const char *data)
{
	FIO_PRINTF(FIO_LOG, "PRIVMSG %s :%s\r\n", channel, data);
	return sck_sendf(s, "PRIVMSG %s :%s\r\n", channel, data);
}

