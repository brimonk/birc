#include "common.h"
#include "irc.h"
#include "rpg.h"
#include "rpg_quest.h"
#include "monsters.h"
#include "items.h"

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

	i32 rand_value = rand() % 4;
	i32 success = rand_value >= 2;
	i32 item_event = (rand_value == 1 || rand_value == 2);
	i32 gp = 0, xp = 0;

	LOG("rand_value %d, success %d, item_event %d", rand_value, success, item_event);

	if (success) {
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
		if (player->item.flags & ITEM_FLAGS_INUSE) {
			snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
				" Their %s %s is used to great effect!", player->item.adjective, player->item.kind);
		} else {
			if (item_event && !(player->item.flags & ITEM_FLAGS_INUSE)) {
				ITEMS_GenerateItem(player);
				snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
					" On their quest, they find a %s %s!",
					player->item.adjective, player->item.kind);
			}
		}

		snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
			" They gain %dXP and %dGP%s!", xp, gp, slevel != elevel ? " (LEVEL UP)" : "");
	} else {
		if (item_event && (player->item.flags & ITEM_FLAGS_INUSE)) {
			snprintf(msg + strlen(msg), sizeof(msg) - strlen(msg),
				" In the process, they lose their %s!", player->item.kind);
			ITEMS_ClearItem(player);
		}
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
