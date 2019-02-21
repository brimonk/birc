/*
 * Brian Chrzanowski
 * Wed Feb 20, 2019 03:04
 *
 * Brian Chrzanowski's IRC Bot
 *
 * TODO (Brian)
 * 1. Setup Module Loading
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dlfcn.h>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>

#include "irc.h"
#include "fio.h"

#define MAXMODS 16
#define DEFAULTMODDIR "./mod"

int loadmoddir(const char *dir, struct modtable_t *tab, int tablen);
void *loadmod(const char *file);

int main(int argc, char **argv)
{
	struct modtable_t modtable[MAXMODS];
	irc_t irc;

	fio_setfp(fopen("birc_log.txt", "a"));

	FIO_PRINTF(FIO_ERR, "More Newlines\n");

	loadmoddir(DEFAULTMODDIR, modtable, MAXMODS);

	if (irc_connect(&irc, "irc.freenode.org", "6667") < 0) {
		fprintf(stderr, "Connection failed.\n");
		goto exit_err;
	}

	irc_set_output(&irc, "output");

	if (irc_login(&irc, "brimonk_testbot") < 0) {
		fprintf(stderr, "Couldn't log in.\n");
		goto exit_err;
	}

	if (irc_join_channel(&irc, "#testingbot") < 0) {
		fprintf(stderr, "Couldn't join channel.\n");
		goto exit_err;
	}

	while (irc_handle_data(&irc) >= 0);

	irc_close(&irc);
	fio_closefp();

	return 0;

exit_err:
	irc_close(&irc);
	fio_closefp();
	return 1;
}

/* loadmoddir : load all modules in a directory */
int loadmoddir(const char *dir, struct modtable_t *tab, int tablen)
{
	DIR *moddir;
	struct dirent *currdir;
	int i;

	moddir = opendir(dir);

	if (!moddir) {
		FIO_PRINTF(FIO_ERR, "Couldn't open %s : %s", dir, strerror(errno));
		return 0;
	}

	for (i = 0; i < tablen; i++) {
		errno = 0;
		currdir = readdir(moddir); /* read the dir */

		if (!currdir)
			break;

		tab[i].path = strdup(&currdir->d_name[0]);
		tab[i].func = loadmod(currdir->d_name);

		if (!tab[i].func)
			FIO_PRINTF(FIO_ERR, "Couldn't load shared lib: %s", dlerror());
	}

	if (!currdir && errno) {
		FIO_PRINTF(FIO_ERR, "directory read error: %s", strerror(errno));
	}

	closedir(moddir);

	if (i == tablen) {
		FIO_PRINTF(FIO_WRN, "not all modules loaded\n");
	}

	return i;
}

void *loadmod(const char *file)
{
#define MODULEENTRY "birc_entry"
	void *ptr;

	ptr = dlopen(file, RTLD_LAZY);

	return ptr;
}

void cleanmod(struct modtable_t *tab, int tablen)
{
	int i;

	for (i = 0; i < tablen; i++) {
		if (tab[i].path)
			free(tab[i].path);
		if (tab[i].func)
			dlclose(tab[i].path);
	}
}

