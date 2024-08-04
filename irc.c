/*
 * Brian Chrzanowski
 * Wed Feb 20, 2019 03:07
 *
 * IRC Library Functions
 *
 * TODO (Brian)
 *
 * * put the actual IRC command handling somewhere else
 *   this file is supposed to be a library for easy IRC handling; however, it's
 *   somewhat intermingled with the logging module, as well as it has an assload
 *   of specific to my application commands
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include <unistd.h>
#include <errno.h>

#include "socket.h"
#include "irc.h"
#include "common.h"
#include "stringext.h"

#include "rpg.h"
#include "rpg_quest.h"
#include "rpg_fish.h"
#include "timer.h"

#define WAKEUP_WORD "!rpg"

/* function declarations */
static int irc_botcmd_help(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_stats(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_work(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_quest(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_fish(irc_t *irc, char *irc_nick, char *arg);
static int irc_botcmd_ping(irc_t *irc, char *irc_nick, char *arg);

struct ircfunc_t {
	char *command;
	char *usage;
	int (*func)(irc_t *, char *, char *);
};

static struct ircfunc_t ircfuncs[] = {
	{"help",      "USAGE: " WAKEUP_WORD " help <command>",   irc_botcmd_help},
	{"stats",     "USAGE: " WAKEUP_WORD " stats <command>",  irc_botcmd_stats},
	{"work",      "USAGE: " WAKEUP_WORD " work",             irc_botcmd_work},
	{"quest",     "USAGE: " WAKEUP_WORD " quest",            irc_botcmd_quest},
	{"fish",      "USAGE: " WAKEUP_WORD " fish",             irc_botcmd_fish},
	{"ping",      "USAGE: " WAKEUP_WORD " ping",             irc_botcmd_ping},
};

struct strdict_t {
	char *key;
	char *val;
};

/* irc_connect : connect to an irc server */
int irc_connect(irc_t *irc, const char* server, const char* port)
{
	if ((irc->s = get_socket(server, port)) < 0) {
		return -1;
	}

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
	int rc, i, bufidx;

	/* wait for and receive data from the server */
	if ((rc = sck_recv(irc->s, tempbuffer, sizeof(tempbuffer) - 2)) <= 0) {
		ERR("Got -1 From Socket %s", strerror(errno));
		return -1;
	}

	tempbuffer[rc] = '\0';
	bufidx = 0;

	for (i = 0; i < rc; i++) {
		switch (tempbuffer[i]) {
		case '\r':
		case '\n':
			irc->servbuf[bufidx] = '\0';

			if (strlen(irc->servbuf) == 0) {
				return 0;
			}

#if 0
			DBG("%s", irc->servbuf);
#endif

			if (irc_parse_action(irc) < 0)
				return -1;

			break;

		default:
			irc->servbuf[bufidx] = tempbuffer[i];
			if (bufidx >= (sizeof(irc->servbuf) -1))
				; // Overflow!
			else
				bufidx++;
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
		ERR("We don't know how to handle NOTICE AUTH...");
		return 0;

	} else if (strncmp(irc->servbuf, "ERROR :", 7) == 0) {
		/* log the fact that the server sent us an error and move on */
		ERR("We don't know how to handle ERROR...");
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
	// First, we check if we have a record for this player.
	Player *player = RPG_FindByNickname(irc_nick);
	if (player == NULL) {
		player = RPG_AddPlayer(irc_nick);
	}

	if (*msg == '!') { /* if we have a thing formatted like a command... */
		// then we can parse it out
		char *rpg = strtok(msg, " ");
		char *command = strtok(NULL, " ");
		char *arg = strtok(NULL, "");
		while (arg && isblank(*arg))
			arg++;

		if (streq(rpg, WAKEUP_WORD)) { // !rpg, etc.
			for (size_t i = 0; i < ARRSIZE(ircfuncs); i++) {
				if (strcmp(command, ircfuncs[i].command) == 0) {
					return ircfuncs[i].func(irc, irc_nick, arg);
				}
			}
		}
	} else { /* non command stuff */
		// return irc_bot_banter(irc, irc_nick, msg);
	}

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

/* irc_botcmd_ping : responds to a user with "pong" */
static int irc_botcmd_ping(irc_t *irc, char *irc_nick, char *arg)
{
	if (irc_msg(irc->s, irc->channel, "pong") < 0)
		return -1;

	return 0;
}

// irc_botcmd_stats : handles help command and prints command usage info
static int irc_botcmd_stats(irc_t *irc, char *irc_nick, char *arg)
{
	Player *player = RPG_FindByNickname(irc_nick);
	assert(player != NULL);

	char msg[512] = {0};

	snprintf(msg, sizeof msg, "Stats for '%s': LVL %d (%ldXP), %ld GP",
		player->nickname, RPG_GetLevel(player), player->xp, player->gp
	);
	return irc_msg(irc->s, irc->channel, msg);
}

IRCFutureContext *GetFutureContext(irc_t *irc, char *nickname)
{
	IRCFutureContext *ctx = calloc(1, sizeof(*ctx));
	strncpy(ctx->nickname, nickname, sizeof ctx->nickname);
	ctx->irc = irc;
	return ctx;
}

static void *irc_botcmd_work_completed(void *ptr)
{
	char msg[512] = {0};

	IRCFutureContext *ctx = ptr;

	Player *player = RPG_UnlockWithNickname(ctx->nickname);
	if (player == NULL) {
		DBG("ctx->nickname [%s] cannot be unlocked!", ctx->nickname);
		assert(false);
	}

	i32 starting_level = RPG_GetLevel(player);

	LOG("%s's WORK COMPLETED!", player->nickname);

	char *jobs[] = {
		"does laundry",
		"forages for food",
		"hunts for food",
		"chops some trees",
		"creates some charcoal",
		"buses tables at the inn",
		"collects taxes",
		"takes a shift as a guard",
		"cooks at the inn",
		"refills tubs at the bath house",

		"tends to the wheat fields",
		"tends to the barley fields",
		"tends to the rice fields",
		"tends to the potato fields",
		"tends to the spice fields",

		"tends to the apple orchards",
		"tends to the date orchards",
		"tends to the pear orchards",

		"tends to flocks of sheep",
		"tends to flocks of cows",
		"tends to flocks of pigs",
		"feeds the chickens",
		"butchers up some sheep",
		"butchers up some cows",
		"butchers up some pigs",
		"butchers up some pigs making hot dogs in the process",
		"butchers up some chickens",

		"chases out wild boar",
		"chases out wild foxes",
		"chases out wild hyenas",

		"works at the sawmill",
		"weaves clothing",
		"carves and puts together furniture",

		"spends time practicing metallurgy",
		"spends time hammering out nails",
		"spends time hammering out hinges",
		"spends time hammering out swords",
		"spends time hammering out axes",
		"spends time hammering out knives",
		"spends time hammering out horseshoes",
	};

	i32 gp, xp, job;

	// TODO dice functions somewhere else?

	gp = rand() % 10 + 1;
	xp = rand() % 10 + 1;
	job = rand() % ARRSIZE(jobs);
	assert(jobs[job] != NULL);

	RPG_AddGP(player, gp);
	RPG_AddXP(player, xp);

	i32 after_level = RPG_GetLevel(player);

	snprintf(msg, sizeof msg, "%s %s, and gains %dXP and %dGP!%s",
		player->nickname, jobs[job], xp, gp,
		starting_level != after_level ? "(LEVEL UP)" : ""
	);

	LOG("%s", msg);
	int rc = irc_msg(ctx->irc->s, ctx->irc->channel, msg);
	LOG("Message send with RC of %d", rc);

	free(ctx);

	return NULL;
}

static int rpg_possibly_start_action(irc_t *irc, char *irc_nick, void *(*fn)(void *), int seconds, char *fmt)
{
	char msg[512] = {0};

	Player *player = RPG_LockWithNickname(irc_nick);
	if (player == NULL) {
		snprintf(msg, sizeof msg, "%s is already doing something!", irc_nick);
	} else {
		timer_fn_enqueue(timer_get_time(seconds, 0), fn, GetFutureContext(irc, irc_nick));
		snprintf(msg, sizeof msg, fmt, irc_nick);
	}

	LOG("%s", msg);
	return irc_msg(irc->s, irc->channel, msg);
}

// irc_botcmd_work : this particular RPG player wants to do some work
static int irc_botcmd_work(irc_t *irc, char *irc_nick, char *arg)
{
	char *fmt = "%s begins to do work for the village...";
	return rpg_possibly_start_action(irc, irc_nick, irc_botcmd_work_completed, 5, fmt);
}

// irc_botcmd_quest : this particular RPG player wants to go on a quest
static int irc_botcmd_quest(irc_t *irc, char *irc_nick, char *arg)
{
	char msg[512];
	rpg_quest_generate(msg, sizeof msg);
	return rpg_possibly_start_action(irc, irc_nick, irc_botcmd_quest_completed, 5, msg);
}

// irc_botcmd_fish : this particular RPG player wants to go on a quest
static int irc_botcmd_fish(irc_t *irc, char *irc_nick, char *arg)
{
	char *fmt = "%s goes fishing...";
	return rpg_possibly_start_action(irc, irc_nick, irc_botcmd_quest_completed, 5, fmt);
}

int irc_log_message(irc_t *irc, const char* nick, const char* message)
{
	char timestring[128];
	time_t curtime;

	time(&curtime);
	strftime(timestring, 127, "%F - %H:%M:%S", localtime(&curtime));
	timestring[127] = '\0';

	MSG("%s [%s] <%s> %s\n", irc->channel, timestring, nick, message);

	return 0;
}

void irc_close(irc_t *irc)
{
	close(irc->s);
}

// irc_pong : answers pong requests
int irc_pong(int s, const char *data)
{
	return sck_sendf(s, "PONG :%s\r\n", data);
}

// irc_reg : registers user upon login
int irc_reg(int s, const char *nick, const char *username, const char *fullname)
{
	return sck_sendf(s, "NICK %s\r\nUSER %s localhost 0 :%s\r\n", nick, username, fullname);
}

// irc_join : joins channels
int irc_join(int s, const char *data)
{
	return sck_sendf(s, "JOIN %s\r\n", data);
}

// irc_part : sends the PART command to the server
int irc_part(int s, const char *data)
{
	return sck_sendf(s, "PART %s\r\n", data);
}

// irc_nick : changes irc nickname
int irc_nick(int s, const char *data)
{
	return sck_sendf(s, "NICK %s\r\n", data);
}

// irc_quit : quits irc
int irc_quit(int s, const char *data)
{
	return sck_sendf(s, "QUIT :%s\r\n", data);
}

// irc_topic : sets/removes the topic of a channel
int irc_topic(int s, const char *channel, const char *data)
{
	return sck_sendf(s, "TOPIC %s :%s\r\n", channel, data);
}

// irc_action : executes an action (.e.g /me is hungry)
int irc_action(int s, const char *channel, const char *data)
{
	int rc;
	rc = sck_sendf(s, "PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
	DBG("PRIVMSG %s :\001ACTION %s\001\r\n", channel, data);
	return rc;
}

// irc_msg : sends a channel message or a query
int irc_msg(int s, const char *channel, const char *data)
{
	int rc;
	rc = sck_sendf(s, "PRIVMSG %s :%s\r\n", channel, data);
	DBG("PRIVMSG %s :%s\r\n", channel, data);
	return rc;
}
