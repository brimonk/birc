/*
 * Brian Chrzanowski
 * Wed Feb 20, 2019 03:07
 *
 * IRC Library Functions
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>

#include "socket.h"
#include "irc.h"
#include "fio.h"
#include "common.h"

static int url_encode(char *buf, int buflen, char *src, char *prefix);
static int url_encode_byte(unsigned char in);

/* function declarations */
static int irc_botcmd_help(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_ping(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_smack(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_google(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_wiki(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_8ball(irc_t *irc, char *irc_nick, char *arg);

static int irc_bot_banter(irc_t *irc, char *irc_nick, char *arg);

struct ircfunc_t {
	char *command;
	char *usage;
	int (*func)(irc_t *, char *, char *);
};

static struct ircfunc_t ircfuncs[] = {
	{"help",   "USAGE: !help <command>",   irc_botcmd_help},
	{"ping",   "USAGE: !ping",             irc_botcmd_ping},
	{"smack",  "USAGE: !smack <person>",   irc_botcmd_smack},
	{"google", "USAGE: !google <search>",  irc_botcmd_google},
	{"8ball",  "USAGE: !8ball <question>", irc_botcmd_8ball},
	{"wiki",   "USAGE: !wiki <search>",    irc_botcmd_wiki}
};



#define WEBPREFIX_GOOGLE "https://www.google.com/search?q="
#define WEBPREFIX_GITHUB "https://github.com/search?q="


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

#if 0
			FIO_PRINTF(FIO_LOG, "%s", irc->servbuf);
#endif

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

	/* if we managed to get here, see if we can get off some witty banter */
	return irc_bot_banter(irc, irc_nick, arg);
}

static int irc_bot_banter(irc_t *irc, char *irc_nick, char *arg)
{
	FIO_PRINTF(FIO_VER, "irc_bot_banter not implemented yet\n");
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

	if (arg) { /* we got an arg, find the one we need, and print the help */
		for (i = 0; i < ARRSIZE(ircfuncs); i++) {
			if (strcmp(arg, ircfuncs[i].command) == 0)
				break;
		}

		if (i == ARRSIZE(ircfuncs)) {
			snprintf(buf, sizeof(buf),
					"%s: \"%s\" isn't a command", irc_nick, arg);
		} else {
			snprintf(buf, sizeof(buf), "%s: %s", irc_nick, ircfuncs[i].usage);
		}


	} else { /* no arg, print out all of the commands that we can fit in here */
		snprintf(buf, sizeof(buf), "%s: commands: ", irc_nick);
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
	/*
	 * Similar to the google command, this command generates a github query to
	 * search the RetropieWiki.
	 *
	 * Github has special search rules, which can be found here:
	 *     https://help.github.com/en/articles/searching-wikis
	 * The jist is that we're setting
	 *     user:retropie
	 *     repo:retropie-setup
	 *     in:title
	 *     in:body
	 *
	 * Then plop the query string after it, and encode the URL
	 * https://github.com/search?q=user%3Aretropie+
	 *                repo%3ARetroPie-Setup+in%3Atitle+Nintendo+64&type=Wikis
	 */

	int rc;
	char mesg[512], tmpbuf[512];
	memset(mesg, 0, sizeof(mesg));
	memset(tmpbuf, 0, sizeof(tmpbuf));

	snprintf(tmpbuf, sizeof(mesg),
			"user:retropie+" "repo:RetroPie-Setup+"
			"in:title+%s&type=Wikis", arg);

	rc = url_encode(mesg, sizeof(mesg), tmpbuf, WEBPREFIX_GITHUB);

	if (rc < 0) {
		FIO_PRINTF(FIO_ERR, "Error Converting %s to proper URL", tmpbuf);
		snprintf(mesg, sizeof(mesg), "Error Converting input to proper URL...");
	}

	return irc_msg(irc->s, irc->channel, mesg);
}

/* irc_botcmd_8ball : responds to magic 8 ball requests */
static int irc_botcmd_8ball(irc_t *irc, char *irc_nick, char *arg)
{
	/*
	 * You'd think there were only 8 answers inside of a magic 8 ball, but it
	 * turns out there's like, 20! Who knew!
	 */

	char *table[] = {
		"It is certain.", "It is decidedly so.", "Without a doubt",
		"Yes - definitely.", "You may rely on it.", "As I see it, yes.",
		"Most likely.", "Outlook good.", "Yes.",
		"Signs point to yes.", "Reply hazy, try again.", "Ask again later",
		"Better not tell you now", "Cannot predict now",
		"Concentrate and ask again", "Don't count on it.", "My reply is no.",
		"My sources say no.", "Outlook not so good.", "Very doubtful."
	};

	int i;

	i = rand() % ARRSIZE(table);

	if (irc_msg(irc->s, irc->channel, table[i]) < 0)
		return -1;

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
	int rc;
	char mesg[512];

	memset(mesg, 0, sizeof(mesg)); /* clean the buffer */

	if (!arg) {
		return 0;
	}

	rc = url_encode(mesg, sizeof(mesg), arg, WEBPREFIX_GOOGLE);

	if (rc < 0) {
		snprintf(mesg, sizeof(mesg), "Search too long. Google it youself!");
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
	int rc;
	rc = sck_sendf(s, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
	FIO_PRINTF(FIO_LOG, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
	return rc;
}

/* irc_msg : sends a channel message or a query */
int irc_msg(int s, const char *channel, const char *data)
{
	int rc;
	rc = sck_sendf(s, "PRIVMSG %s :%s\r\n", channel, data);
	FIO_PRINTF(FIO_LOG, "PRIVMSG %s :%s\r\n", channel, data);
	return rc;
}

/* misc */

/* url_encode : encodes a URL query string to a web friendly format */
static int url_encode(char *buf, int buflen, char *src, char *prefix)
{
	int len;

	snprintf(buf, buflen, "%s", prefix); /* plop the query prefix first */

	/* then encode the rest of the URL */
	for (len = strlen(buf); len < buflen && *src; src++, len = strlen(buf)) {
		if (url_encode_byte(*src)) { /* encoding */
			snprintf(buf + len, buflen-len, "%%%02x", *src);
		} else { /* no encoding */
			snprintf(buf + len, buflen-len, "%c", *src);
		}
	}

	if (*src != '\0') {
		return -1; /* couldn't encode the url, not enough space */
	}

	return 0;
}

/* url_encode_byte : determine if this byte needs to be encoded specially */
static int url_encode_byte(unsigned char in)
{
	int rc;

	rc = 1; /* assume we're going to encode it */

	if (isdigit(in) || isalpha(in)) {
		rc = 0;
	}

	/* until we assume we don't want to */

	switch (in) {
	case '+':
	case '/':
	case '&':
	case '=':
		rc = 0;
		break;
	default:
		break;

	}

	return rc;
}

