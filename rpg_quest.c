#include "common.h"
#include "irc.h"
#include "rpg.h"
#include "rpg_quest.h"
#include "monsters.h"

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

// rpg_quest_generate: writes a quest into the buffer 's'
void rpg_quest_generate(char *s, size_t slen)
{
	snprintf(s, slen, "%%s goes on a quest to %s the/a %s...",
		MONSTER_GetMonsterQuestAction(),
		MONSTER_GetRandomMonsterName()
	);
}
