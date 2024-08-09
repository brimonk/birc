#include "common.h"
#include "rpg.h"
#include "items.h"

static char *ITEM_KIND[] = {
	"SWORD",
	"MACE",
	"HAMMER",
	"AXE",
	"WAND",
	"HOLYSYMBOL",
	"PRAYERBOOK",
	"STAFF",
	"SPELLBOOK",
};

static char *ITEM_ADJECTIVES[] = {
	"ANCIENT",
	"ARCANE",
	"BLESSED",
	"CURSED",
	"DIVINE",
	"ELDRITCH",
	"ENCHANTED",
	"EPIC",
	"ETERNAL",
	"EXOTIC",
	"FLAMING",
	"FROSTY",
	"GHOSTLY",
	"GLIMMERING",
	"GLOWING",
	"GOLDEN",
	"GRIM",
	"HAUNTED",
	"HELLISH",
	"HOLY",
	"ILLUMINATED",
	"IMMORTAL",
	"IMPERIAL",
	"INFUSED",
	"JAGGED",
	"LUMINOUS",
	"LURID",
	"MAGICAL",
	"MYSTICAL",
	"MYTHIC",
	"OBSIDIAN",
	"OCCULT",
	"OTHERWORLDLY",
	"PHANTASMAL",
	"PRIMAL",
	"PROPHETIC",
	"RADIANT",
	"RARE",
	"RUNED",
	"SACRED",
	"SAVAGE",
	"SHADOWY",
	"SHIMMERING",
	"SILVERED",
	"SPECTRAL",
	"SPELLBOUND",
	"SPIRITUAL",
	"STELLAR",
	"STORMY",
	"TERRIFYING",
	"TITANIC",
	"TWILIGHT",
	"ULTIMATE",
	"UNDYING",
	"UNHOLY",
	"VENGEFUL",
	"VENERABLE",
	"WARPED",
	"WICKED",
	"WONDROUS",
	"ABYSSAL",
	"ALCHEMICAL",
	"BATTLEWORN",
	"BLOODY",
	"BRILLIANT",
	"CALMING",
	"CELESTIAL",
	"CHARGED",
	"CLOAKED",
	"DARKENED",
	"DIMENSIONAL",
	"DREADFUL",
	"EMBERED",
	"FIENDISH",
	"GLACIAL",
	"HARMONIOUS",
	"HEROIC",
	"HONORABLE",
	"INSCRIBED",
	"LEGENDARY",
	"MAGNETIC",
	"MALEFIC",
	"MARTIAL",
	"NIGHTMARISH",
	"OMINOUS",
	"PIERCING",
	"RELIC",
	"SORCEROUS",
	"THUNDEROUS",
	"TRANSCENDENT",
	"VIGILANT",
	"VIVID",
	"WARLOCK",
	"WINDWORN",
	"WRATHFUL",
	"ZEALOUS",
};

typedef struct ItemDef {
	i32 type;
	i32 adjective;
} ItemDef;

void ITEMS_ClearItem(Player *player)
{
	memset(&player->item, 0, sizeof(player->item));
}

void ITEMS_GenerateItem(Player *player)
{
	// TODO add optionally generated names :)

	ITEMS_ClearItem(player);

	char *kind = ITEM_KIND[rand() % ARRSIZE(ITEM_KIND)];
	char *adjective = ITEM_ADJECTIVES[rand() % ARRSIZE(ITEM_ADJECTIVES)];

	player->item.flags |= ITEM_FLAGS_INUSE;
	strncpy(player->item.kind, kind, sizeof(player->item.kind));
	strncpy(player->item.adjective, adjective, sizeof(player->item.adjective));
}
