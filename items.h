#ifndef ITEMS_H
#define ITEMS_H

#include "rpg.h"

#define ITEM_FLAGS_INUSE  0x01
#define ITEM_FLAGS_NAMED  0x02

typedef struct Player Player;

typedef struct Item {
	char kind[32];
	char adjective[32];
	char name[32];
	u64 flags;
} Item;

// ITEMS_ClearItem: clears the item on the player
void ITEMS_ClearItem(Player *player);

// ITEMS_GenerateItem: reset the item on the player, and create a new one
void ITEMS_GenerateItem(Player *player);

#endif // ITEMS_H
