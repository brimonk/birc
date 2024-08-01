#ifndef RPG_H
#define RPG_H

#include "common.h"

enum {
	STAT_STR,
	STAT_DEX,
	STAT_CON,
	STAT_INT,
	STAT_WIS,
	STAT_CHA,
	STAT_TOTAL
};

typedef struct Player {
	i32 hp;
	i32 stats[STAT_TOTAL];
} Player;

typedef struct Spell {
	char *name;
} Spell;

#endif // RPG_H
