/*
 * Brian Chrzanowski
 * Wed Feb 20, 2019 18:54
 *
 * File IO Handling
 */

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include "fio.h"

static char buf[512];
static FILE *modfp = NULL;

FILE *fio_getstaticfp()
{
	return modfp;
}

/* fio_setfp : sets the module file pointer to fp, and sets line buffering */
void fio_setfp(FILE *fp)
{
	if (fp) {
		modfp = fp;
		setvbuf(modfp, buf, _IOLBF, sizeof(buf));
	}
}

void fio_closefp()
{
	fclose(modfp);
}

/* fio_printf : printf to our specific, "staticish" FILE *ptr */
int fio_printf(char *file, int line, int level, char *fmt, ...)
{
	va_list args;
	char *ptr;
	int rc;

	rc = 0;

	if (!modfp)
		return rc;

	if (strlen(fmt) == 0)
		return rc;

	va_start(args, fmt); /* get the arguments from the stack */

	switch (level) {
	case FIO_MSG:
		rc = fprintf(modfp, "%s:%d MSG ", file, line);
		break;

	case FIO_WRN:
		rc = fprintf(modfp, "%s:%d WRN ", file, line);
		break;

	case FIO_ERR:
		rc = fprintf(modfp, "%s:%d ERR ", file, line);
		break;

	case FIO_VER:
		rc = fprintf(modfp, "%s:%d VER ", file, line);
		break;

	case FIO_NON:
		/* NON doesn't print any debug messages */
		break;

	default:
		rc = fprintf(modfp, "%s:%d UNKNOWN ", file, line);
		break;
	};

	/* print the rest of the message */
	rc += vfprintf(modfp, fmt, args);

	va_end(args); /* cleanup stack arguments */

	/* writes a newline if the last character of the format wasn't a newline */
	ptr = fmt + strlen(fmt) - 1;
	if (*ptr != '\n') {
		fprintf(modfp, "\n");
	}

	return rc;
}

/* fio_lines : get the number of lines in a FILE *fp */
int fio_lines(FILE *fp)
{
	int lines, c;

	if (!fp)
		return -1;

	rewind(fp);

	/* read lines until you run out */
	for (lines = 0, c = getc(fp); c != EOF; c = getc(fp)) {
		if (c == '\n')
			lines++;
	}

	return lines;
}

/* fio_getline : gets linenumber line from fp storing buflen chars in buf */
int fio_getline(FILE *fp, char *buf, int buflen, int line)
{
	char *ptr;
	int curr;
	char tmpbuf[256];

	if (!fp)
		return -1;

	rewind(fp);
	curr = 0;

	while ((ptr = fgets(tmpbuf, sizeof(tmpbuf), fp)) == tmpbuf) {
		if (curr++ == line)
			break;
	}

	/* have to see if the file has that line */
	if (ptr == tmpbuf) {
		curr = strlen(tmpbuf);
		strncpy(buf, tmpbuf, curr > buflen ? buflen : curr);
	} else {
		return -1;
	}

	return 0;
}

