/*
 * Brian Chrzanowski
 * Mon Oct 01, 2018 20:00
 *
 * String extension functions. Things like regular expression parsing
 * regular expression functions taken and modified from
 * "The Practice of Programming"
 */

#include <ctype.h>
#include <string.h>
#include "string.h"

static int re_matchhere(char *regexp, char *text);
static int re_matchstar(int c, char *regexp, char *text);

/* re_match : search for regexp anywhere in text */
int re_match(char *regexp, char *text)
{
	if (regexp[0] == '^') {
		return re_matchhere(regexp + 1, text);
	}

	do {
		if (re_matchhere(regexp, text))
			return 1;
	} while (*text++ != '\0');

	return 0;
}

/* re_matchhere : search for regex at the beginning of text */
static int re_matchhere(char *regexp, char *text)
{
	if (regexp[0] == '\0')
		return 1;

	if (regexp[1] == '*')
		return re_matchstar(regexp[0], regexp + 2, text);

	if (regexp[0] == '$' && regexp[1] == '\0')
		return *text == '\0';

	if (*text != '\0' && (regexp[0] == '.' || regexp[0] == *text))
		return re_matchhere(regexp + 1, text + 1);
	return 0;
}

/* re_matchstar : recursively match a star character */
static int re_matchstar(int c, char *regexp, char *text)
{
	do { // a * matches zero or more instances
		if (re_matchhere(regexp, text))
			return 1;
	} while (*text != '\0' && (*text++ == c || c == '.'));

	return 0;
}

/* bstrtok : Brian's (Better) strtok */
char *bstrtok(char **str, char *delim)
{
	/*
	 * strtok is super gross. let's make it better (and worse)
	 *
	 * few things to note
	 *
	 * To use this properly, pass a pointer to your buffer as well as a string
	 * you'd like to delimit your text by. When you've done that, you
	 * effectively have two return values. The NULL terminated C string
	 * explicitly returned from the function, and the **str argument will point
	 * to the next section of the buffer to parse. If str is ever NULL after a
	 * call to this, there is no new delimiting string, and you've reached the
	 * end
	 */

	char *ret, *work;

	ret = *str; /* setup our clean return value */
	work = strstr(*str, delim); /* now do the heavy lifting */

	if (work) {
		/* we ASSUME that str was originally NULL terminated */
		memset(work, 0, strlen(delim));
		work += strlen(delim);
	}

	*str = work; /* setting this will make *str NULL if a delimiter wasn't found */

	return ret;
}

/* strisupper : returns true if the whole string is upper case */
int strisupper(char *str)
{
	for (; *str; str++) {
		if (isalpha(*str) && !isupper(*str))
			return 0;
	}

	return 1;
}

