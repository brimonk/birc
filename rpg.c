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
	new.level = 1;

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
