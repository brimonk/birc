#include "common.h"
#include "irc.h"
#include "rpg.h"
#include "rpg_fish.h"

typedef struct Fish Fish;

// RPG_FishRarityRandom: returns a random rarity, using a small D20ish table
int RPG_FishRarityRandom(void);

// RPG_FishRandomByRarity: returns a random fish in the rarity selected
Fish *RPG_FishRandomByRarity(int rarity);

// RPG_FishRarityString: returns a string describing the rarity
char *RPG_FishRarityString(int rarity);

enum {
	FISH_RARITY_NONE,
	FISH_RARITY_COMMON,
	FISH_RARITY_FAIRLYCOMMON,
	FISH_RARITY_UNCOMMON,
	FISH_RARITY_SCARCE,
	FISH_RARITY_RARE,
	FISH_RARITY_EPIC,
	FISH_RARITY_LEGENDARY,
	FISH_RARITY_TOTAL
};

typedef struct Fish {
	char *name;
	char *location;
	i32 rarity;
	i32 gp;
} Fish;

Fish G_FISH_TABLE[] = {
	// FISH_RARITY_NONE
	{ NULL, NULL, FISH_RARITY_NONE, 0 },

	// FISH_RARITY_COMMON
	{"Anchovy", "Sea", FISH_RARITY_COMMON, 2},
	{"Bitterling", "River", FISH_RARITY_COMMON, 9},
	{"Black Bass", "River", FISH_RARITY_COMMON, 4},
	{"Carp", "Pond", FISH_RARITY_COMMON, 3},
	{"Catfish", "Pond", FISH_RARITY_COMMON, 8},
	{"Crucian Carp", "River", FISH_RARITY_COMMON, 1},
	{"Dab", "Sea", FISH_RARITY_COMMON, 3},
	{"Dace", "River", FISH_RARITY_COMMON, 2},
	{"Frog", "Pond", FISH_RARITY_COMMON, 12},
	{"Killifish", "Pond", FISH_RARITY_COMMON, 3},
	{"Loach", "River", FISH_RARITY_COMMON, 4},
	{"Pale Chub", "River", FISH_RARITY_COMMON, 2},
	{"Pond Smelt", "River", FISH_RARITY_COMMON, 4},
	{"Sea Bass", "Sea", FISH_RARITY_COMMON, 4},
	{"Tadpole", "Pond", FISH_RARITY_COMMON, 1},
	{"Tilapia", "River", FISH_RARITY_COMMON, 8},
	{"Yellow Perch", "River", FISH_RARITY_COMMON, 4},

	// FISH_RARITY_FAIRLYCOMMON
	{"Butterfly Fish", "Sea", FISH_RARITY_FAIRLYCOMMON, 10},
	{"Clown Fish", "Sea", FISH_RARITY_FAIRLYCOMMON, 6},
	{"Freshwater Goby", "River", FISH_RARITY_FAIRLYCOMMON, 4},
	{"Goldfish", "Pond", FISH_RARITY_FAIRLYCOMMON, 13},
	{"Moray Eel", "Sea", FISH_RARITY_FAIRLYCOMMON, 20},
	{"Olive Flounder", "Sea", FISH_RARITY_FAIRLYCOMMON, 8},
	{"Red Snapper", "Sea", FISH_RARITY_FAIRLYCOMMON, 30},
	{"Sea Butterfly", "Sea", FISH_RARITY_FAIRLYCOMMON, 10},
	{"Sea Horse", "Sea", FISH_RARITY_FAIRLYCOMMON, 6},
	{"Squid", "Sea", FISH_RARITY_FAIRLYCOMMON, 5},
	{"Suckerfish", "Sea", FISH_RARITY_FAIRLYCOMMON, 15},
	{"Surgeonfish", "Sea", FISH_RARITY_FAIRLYCOMMON, 10},
	{"Sweetfish", "River", FISH_RARITY_FAIRLYCOMMON, 9},

	// FISH_RARITY_UNCOMMON
	{"Angelfish", "River", FISH_RARITY_UNCOMMON, 30},
	{"Betta", "River", FISH_RARITY_UNCOMMON, 25},
	{"Char", "River (Clifftop)", FISH_RARITY_UNCOMMON, 38},
	{"Cherry Salmon", "River (Clifftop)", FISH_RARITY_UNCOMMON, 10},
	{"Guppy", "River", FISH_RARITY_UNCOMMON, 13},
	{"Koi", "Pond", FISH_RARITY_UNCOMMON, 40},
	{"Mitten Crab", "River", FISH_RARITY_UNCOMMON, 40},
	{"Neon Tetra", "River", FISH_RARITY_UNCOMMON, 5},
	{"Nibble Fish", "River", FISH_RARITY_UNCOMMON, 15},
	{"Pike", "River", FISH_RARITY_UNCOMMON, 18},
	{"Piranha", "River", FISH_RARITY_UNCOMMON, 25},
	{"Pop-Eyed Goldfish", "Pond", FISH_RARITY_UNCOMMON, 13},
	{"Puffer Fish", "Sea", FISH_RARITY_UNCOMMON, 25},
	{"Rainbowfish", "River", FISH_RARITY_UNCOMMON, 8},
	{"Ranchu Goldfish", "Pond", FISH_RARITY_UNCOMMON, 45},
	{"Ray", "Sea", FISH_RARITY_UNCOMMON, 30},
	{"Ribbon Eel", "Sea", FISH_RARITY_UNCOMMON, 10},
	{"Saddled Bichir", "River", FISH_RARITY_UNCOMMON, 40},
	{"Salmon", "River (Mouth)", FISH_RARITY_UNCOMMON, 7},
	{"Soft-Shelled Turtle", "River", FISH_RARITY_UNCOMMON, 37},

	// FISH_RARITY_SCARCE
	{"Barred Knifejaw", "Sea", FISH_RARITY_SCARCE, 50},
	{"Ocean Sunfish", "Sea", FISH_RARITY_SCARCE, 40},

	// FISH_RARITY_RARE
	{"Arapaima", "River", FISH_RARITY_RARE, 100},
	{"Arowana", "River", FISH_RARITY_RARE, 100},
	{"Barreleye", "Sea", FISH_RARITY_RARE, 120},
	{"Blue Marlin", "Sea", FISH_RARITY_RARE, 100},
	{"Dorado", "River", FISH_RARITY_RARE, 150},
	{"Gar", "Pond", FISH_RARITY_RARE, 90},
	{"Giant Snakehead", "Pond", FISH_RARITY_RARE, 55},
	{"Giant Trevally", "Pier", FISH_RARITY_RARE, 120},
	{"Golden Trout", "River (Clifftop)", FISH_RARITY_RARE, 150},
	{"Hammerhead Shark", "Sea", FISH_RARITY_RARE, 80},
	{"King Salmon", "River (Mouth)", FISH_RARITY_RARE, 90},
	{"Mahi-Mahi", "Pier", FISH_RARITY_RARE, 60},
	{"Napoleonfish", "Sea", FISH_RARITY_RARE, 100},
	{"Oarfish", "Sea", FISH_RARITY_RARE, 90},
	{"Saw Shark", "Sea", FISH_RARITY_RARE, 120},
	{"Sturgeon", "River (Mouth)", FISH_RARITY_RARE, 100},
	{"Stringfish", "River (Clifftop)", FISH_RARITY_RARE, 150},
	{"Tuna", "Pier", FISH_RARITY_RARE, 100},
	{"Whale Shark", "Sea", FISH_RARITY_RARE, 130},

	// FISH_RARITY_EPIC
	{"Coelacanth", "Sea (Rain)", FISH_RARITY_EPIC, 150},
	{"Great White Shark", "Sea", FISH_RARITY_EPIC, 150},

	// FISH_RARITY_LEGENDARY
	{"Celestial Carp", "Pond", FISH_RARITY_LEGENDARY, 500},
	{"Mythic Marlin", "Sea", FISH_RARITY_LEGENDARY, 600},
	{"Phantom Piranha", "River", FISH_RARITY_LEGENDARY, 550}
};

char *G_RARITY_STRINGS[] = {
	NULL,
	"COMMON",
	"FAIRLYCOMMON",
	"UNCOMMON",
	"SCARCE",
	"RARE",
	"EPIC",
	"LEGENDARY"
};

// irc_botcmd_fish_completed: completes a fishing trip for the player
void *irc_botcmd_fish_completed(void *ptr)
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

	LOG("%s's FISHING COMPLETED!", player->nickname);

	i32 gp = 0, xp = 0;

	Fish *fish = RPG_FishRandomByRarity(RPG_FishRarityRandom());

	gp = fish->gp;
	xp = 5;

	RPG_AddGP(player, gp);
	RPG_AddXP(player, xp);

	elevel = RPG_GetLevel(player);

	snprintf(msg, sizeof msg, "%s went fishing in a %s and caught a %s (%s)! They sell it for %dGP, and gain %dXP%s!",
		player->nickname, fish->location, fish->name, RPG_FishRarityString(fish->rarity), gp, xp,
		slevel != elevel ? " (LEVEL UP)" : ""
	);

	LOG("%s", msg);
	int rc = irc_action(ctx->irc->s, ctx->irc->channel, msg);
	LOG("Message send with RC of %d", rc);

	free(ctx);
	return NULL;
}

// RPG_FishRarityRandom: returns a random rarity, using a small D20ish table
int RPG_FishRarityRandom(void)
{
	// TODO Better RNG
	int roll = rand() % 200 + 1;

#define BETWEEN(lo, hi) ((lo) <= roll && roll <= (hi))
	if (BETWEEN(1, 50)) {
		return FISH_RARITY_COMMON;
	} else if (BETWEEN(51, 100)) {
		return FISH_RARITY_FAIRLYCOMMON;
	} else if (BETWEEN(101, 150)) {
		return FISH_RARITY_UNCOMMON;
	} else if (BETWEEN(150, 170)) {
		return FISH_RARITY_SCARCE;
	} else if (BETWEEN(171, 190)) {
		return FISH_RARITY_RARE;
	} else if (BETWEEN(191, 199)) {
		return FISH_RARITY_EPIC;
	} else if (BETWEEN(200, 200)) {
		return FISH_RARITY_LEGENDARY;
	} else {
		return FISH_RARITY_NONE;
	}
#undef BETWEEN
}

// RPG_FishRandomByRarity: returns a random fish in the rarity selected
Fish *RPG_FishRandomByRarity(int rarity)
{
	// NOTE To avoid precomputing the counts of these things in the table, on the first entrance
	// to this function, we actually count up the numbers of fishes by rarity, where they start,
	// etc.

	// TODO test me :)

	struct {
		size_t start, end;
	} rarity_indices[FISH_RARITY_TOTAL] = {
		{  0, 0 },
		{  1, 17 },
		{ 18, 30 },
		{ 31, 50 },
		{ 51, 52 },
		{ 53, 71 },
		{ 72, 73 },
		{ 74, 76 },
	};

	// check the table
	for (size_t i = 0; i < ARRSIZE(rarity_indices); i++) {
		DBG("%ld", i);

		assert(
			G_FISH_TABLE[rarity_indices[i].start].rarity == i &&
			G_FISH_TABLE[rarity_indices[i].end].rarity == i
		);
	}

	size_t start = rarity_indices[rarity].start;
	size_t end = rarity_indices[rarity].end;

	size_t idx = rand() % (end + 1 - start) + start;

	return &G_FISH_TABLE[idx];
}

// RPG_FishRarityString: returns a string describing the rarity
char *RPG_FishRarityString(int rarity)
{
	assert(FISH_RARITY_TOTAL == ARRSIZE(G_RARITY_STRINGS));
	assert(FISH_RARITY_NONE <= rarity && rarity < FISH_RARITY_TOTAL);
	return G_RARITY_STRINGS[rarity];
}
