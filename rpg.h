#ifndef RPG_H
#define RPG_H

// DESIGN - THINGS THAT CANNOT HAPPEN TO A PLAYER
//
// - Be overencumbered
// - Lose XP
// - Have their items/gold stolen
//
// DESIGN - THINGS THAT PLAYERS SHOULD BE ABLE TO DO
//
// - Dungeon Crawling (random chance with loot tables)
// - Raids (with Bosses, multiple people)
// - Get magic items
// - Sell/Buy/Trade items
// - Fish
// - Garden

#include "common.h"
#include "items.h"

typedef struct Player {
	char nickname[64];
	i64 gp;
	i64 xp;
	i32 lock;

	Item item; // single item to avoid text chat bloat
} Player;

void rpg_init(void);
void rpg_free(void);

Player *RPG_FindByNickname(char *nickname);
Player *RPG_AddPlayer(char *nickname);

void RPG_AddGP(Player *player, i64 gp);
void RPG_AddXP(Player *player, i64 gp);

// RPG_LockWithNickname: returns a pointer to the player, locking it in the process
Player *RPG_LockWithNickname(char *nickname);

// RPG_UnlockWithNickname: returns a pointer to the player, unlocking it in the process
Player *RPG_UnlockWithNickname(char *nickname);

i32 RPG_GetLevel(Player *player);

#endif // RPG_H
