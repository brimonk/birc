#ifndef __STRINGEXT
#define __STRINGEXT

/* re_match : search for regexp anywhere in text */
int re_match(char *regexp, char *text);
/* bstrtok : tokenize strings with other strings, reentrantly */
char *bstrtok(char **str, char *delim);
int strisupper(char *str);

#endif
