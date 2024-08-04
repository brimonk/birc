#include "common.h"
#include "irc.h"
#include "rpg.h"
#include "rpg_quest.h"

// irc_botcmd_quest_completed: completes the quest for the user
void *irc_botcmd_quest_completed(void *ptr)
{
	char msg[512] = {0};
	IRCFutureContext *ctx = ptr;

	Player *player = RPG_UnlockWithNickname(ctx->nickname);
	if (player == NULL) {
		DBG("ctx->nickname [%s] cannot be unlocked!", ctx->nickname);
		assert(false);
	}

	int slevel, elevel;

	slevel = RPG_GetLevel(player);

	LOG("%s's QUEST COMPLETED!", player->nickname);

	i32 success = false;
	i32 gp = 0, xp = 0;

	if ((success = rand() % 2) == 1) {
		gp = rand() % 100 + 1;
		xp = rand() % 100 + 1;
	}

	RPG_AddGP(player, gp);
	RPG_AddXP(player, xp);

	elevel = RPG_GetLevel(player);

	snprintf(msg, sizeof msg, "%s %s in their quest!",
		player->nickname, success ? "SUCCEEDS" : "FAILS"
	);

	if (success) {
		snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
			" They gain %dXP and %dGP%s!",
		xp, gp, slevel != elevel ? " (LEVEL UP)" : "");
	}

	LOG("%s", msg);
	int rc = irc_msg(ctx->irc->s, ctx->irc->channel, msg);
	LOG("Message send with RC of %d", rc);

	free(ctx);
	return NULL;
}

// NOTE All of these quests are basically madlibs styled strings of the following format:
//     <PLAYER> goes on a quest in <QUEST PLACE> to <QUEST GOAL>. They <SUCCEED/FAIL>, and gain
//     <XP>XP and <GP>GP.

struct {
	char *place;
	char *goal[4];
} QuestMetadata[] = {
	{
		"the arctic",
		{
			"find and rescue some lost adventurers",
			"bring fuel to a distant outpost",
			"melt a frost giant",
			"put an end to an ice dragon",
		}
	},
	{
		"the artisan district of town",
		{
			"locate a local thief",
			"clear out a group of local drunkards",
			"find where a kobold hideout is",
			"put at stop to the Bywater Barons",
		}
	},
#if 0
	{
		"the castle district",
		{

		}
	},
	{
		"a local cave",
		{
		}
	},
	{
		"some deep tunnels",
		{
		}
	},
	{
		"the far away desert",
		{
			"find the mysterious, wandering merchant",
		}
	},
	{
		"the local forest",
		{
		}
	},
	{
		"close by grasslands",
		{
		}
	},
	{
		"the high district",
		{
		}
	},
	{
		"a far away jungle",
		{
		}
	},
	{
		"the low district",
		{
			"find where a newly born rat king is hiding",
		}
	},
	{
		"the market",
		{
		}
	},
	{
		"a distant mountain",
		{
		}
	},
	{
		"the ocean off the coast",
		{
		}
	},
	{
		"a local river basin",
		{
		}
	},
	{
		"near by ruins",
		{
		}
	},
	{
		"the slums",
		{
		}
	},
	{
		"a near by swamp",
		{
		}
	},
	{
		"the temple district",
		{
		}
	},
	{
		"a near by tomb",
		{
		}
	},
	{
		"the university district",
		{
		}
	},
#endif
};

// rpg_quest_generate: writes a quest into the buffer 's'
void rpg_quest_generate(char *s, size_t slen)
{
	i32 place, goal;

	place = rand() % ARRSIZE(QuestMetadata);
	goal = rand() % ARRSIZE(QuestMetadata[0].goal);

	snprintf(s, slen, "%%s goes on a quest in %s to %s...",
		QuestMetadata[place].place, QuestMetadata[place].goal[goal]
	);
}
