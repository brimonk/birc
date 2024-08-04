#ifndef RPG_QUEST_H
#define RPG_QUEST_H

// irc_botcmd_quest_completed: completes the quest for the player
void *irc_botcmd_quest_completed(void *ptr);

// rpg_quest_generate: writes a quest into the buffer 's'
void rpg_quest_generate(char *s, size_t slen);

#endif // RPG_QUEST_H
