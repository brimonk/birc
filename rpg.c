#include "common.h"
#include "rpg.h"

#define RPG_STORAGE "rpg.db"

typedef struct FileEntryBound {
	char entry[256];
} FileEntryBound;

typedef union FileEntry {
	Player player;
	FileEntryBound anchor;
} FileEntry;

#define MAGIC 0xdeadbeef

typedef struct StorageHeader {
	u64 magic;
	u64 entries;
	u64 entry_size;
	u64 flags;
} StorageHeader;

FileEntry *g_entries = NULL;

void rpg_sync(void);

__attribute__((constructor))
void rpg_init(void)
{
	assert(sizeof(Player) <= sizeof(FileEntryBound));

	FILE *fp = fopen(RPG_STORAGE, "rb");
	if (!fp) {
		arrinsn(g_entries, 0, 64);
		rpg_sync();
	} else {
		StorageHeader header = {0};
		size_t sz;

		sz = fread(&header, sizeof header, 1, fp);
		assert(sz == 1);
		assert(header.magic == MAGIC);
		assert(header.entry_size == sizeof(FileEntryBound));

		arrinsn(g_entries, 0, header.entries);

		sz = fread(g_entries, sizeof(*g_entries), header.entries, fp);
		assert(sz == header.entries);

		fclose(fp);
	}
}

__attribute__((destructor))
void rpg_free(void)
{
	rpg_sync();
	arrfree(g_entries);
}

void rpg_sync(void)
{
	FILE *fp = fopen(RPG_STORAGE, "wb");
	assert(fp != NULL);

	StorageHeader header = {
		.magic = MAGIC,
		.entries = arrlen(g_entries),
		.entry_size = sizeof(FileEntry),
		.flags = 0
	};

	size_t sz = 0;

	sz = fwrite(&header, sizeof header, 1, fp);
	assert(sz == 1);

	sz = fwrite(g_entries, sizeof(*g_entries), arrlen(g_entries), fp);
	assert(sz == arrlen(g_entries));

	fclose(fp);
}

// player_comp: sort players by their names in the list, putting NULL names at the end
int player_comp(const void *a, const void *b)
{
	Player *player_a = (Player *)a;
	Player *player_b = (Player *)b;

	if (player_a->nickname[0] == 0 || player_b->nickname[0] == 0)
		return 1;
	return strcmp(player_a->nickname, player_b->nickname);
}

int player_search_nickname(const void *a, const void *b)
{
	char *nickname = (char *)a;
	Player *player = (Player *)b;

	if (player->nickname[0] == 0)
		return -1;

	return strcmp(nickname, player->nickname);
}

Player *RPG_FindByNickname(char *nickname)
{
	void *p = (Player *)bsearch(nickname, g_entries, arrlen(g_entries), sizeof(*g_entries), player_search_nickname);
	LOG("Finding player '%s' %s %p", nickname, p ? "FOUND" : "NOT FOUND", p);
	return p;
}

Player *RPG_AddPlayer(char *nickname)
{
	Player new = {0};
	strncpy(new.nickname, nickname, sizeof new.nickname);

	FileEntry fe = {0};
	fe.player = new;

	arrput(g_entries, fe);

	qsort(g_entries, arrlen(g_entries), sizeof(*g_entries), player_comp);
	void *p = (Player *)&g_entries[arrlen(g_entries)];

	LOG("Adding player '%s' at address %p", nickname, p);

	for (size_t i = 0; i < arrlen(g_entries); i++) {
		if (((Player *)&g_entries[i])->nickname[0] == 0)
			continue;
		DBG("%ld - %s", i, ((Player *)&g_entries[i])->nickname);
	}

	return p;
}

void RPG_AddGP(Player *player, i64 gp)
{
	player->gp += gp;
}

void RPG_AddXP(Player *player, i64 xp)
{
	player->xp += xp;
}

// source: https://oldschool.runescape.wiki/w/Experience
static i64 G_XP_TABLE[] = {
	0, 83, 174, 276, 388, 512, 650, 801, 969, 1154, 1358, 1584, 1833, 2107, 2411, 2746, 3115,
	3523, 3973, 4470, 5018, 5624, 6291, 7028, 8740, 9730, 10824, 12031, 13363, 14833, 16456, 18247,
	20224, 22406, 24815, 27473, 30408, 33648, 37224, 41171, 45529, 50339, 55649, 61512, 67983,
	75127, 83014, 91721, 111945, 123660, 136594, 150872, 166636, 184040, 203254, 224466, 247886,
	273742, 302288, 333804, 368599, 407015, 449428, 496254, 547953, 605032, 668051, 737627, 814445,
	899257, 992895, 1096278, 1336443, 1475581, 1629200, 1798808, 1986068, 2192818, 2421087,
	2673114, 2951373, 3258594, 3597792, 3972294, 4385776, 4842295, 5346332, 5902831, 6517253,
	7195629, 7944614, 8771558, 9684577, 10692629, 11805606, 13034431
};

i32 RPG_GetLevel(Player *player)
{
	i64 level = 1;

	while (level < ARRSIZE(G_XP_TABLE) && G_XP_TABLE[level] < player->xp) {
		level++;
	}

	return level;
}
