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

#define PRINTTOSTDOUT 1

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
	int rc, tmp;
	char bigbuf[1024];

	rc = 0;

	if (!modfp)
		return rc;

	if (strlen(fmt) == 0)
		return rc;

	memset(bigbuf, 0, sizeof(bigbuf));

	va_start(args, fmt); /* get the arguments from the stack */

	switch (level) {
	case FIO_MSG:
		rc = snprintf(bigbuf, sizeof(bigbuf), "%s:%-4d MSG ", file, line);
		break;

	case FIO_WRN:
		rc = snprintf(bigbuf, sizeof(bigbuf), "%s:%-4d WRN ", file, line);
		break;

	case FIO_ERR:
		rc = snprintf(bigbuf, sizeof(bigbuf), "%s:%-4d ERR ", file, line);
		break;

	case FIO_VER:
		rc = snprintf(bigbuf, sizeof(bigbuf), "%s:%-4d VER ", file, line);
		break;

	case FIO_NON:
		/* NON doesn't print any debug messages */
		break;

	case FIO_LOG:
		rc = snprintf(bigbuf, sizeof(bigbuf), "%s:%-4d LOG ", file, line);
		// rc = snprintf(bigbuf, sizeof(bigbuf), "LOG ");
		break;


	default:
		rc = fprintf(modfp, "%s:%-4d UNKNOWN ", file, line);
		break;
	};

	/* print the rest of the message */
	tmp = strlen(bigbuf);
	rc += vsnprintf(bigbuf + tmp, sizeof(bigbuf) - tmp, fmt, args);

	va_end(args); /* cleanup stack arguments */

	/* writes a newline if the last character of the buffer isn't a newline */
	tmp = strlen(bigbuf);
	ptr = fmt + strlen(fmt) - 1;
	if (*ptr != '\n') {
		sprintf(bigbuf + tmp, "\n");
	}

	/* finally, print the buffer out into our file, and optionally stdout */
	fprintf(modfp, "%s", bigbuf);
#ifdef PRINTTOSTDOUT
	printf("%s", bigbuf);
#endif

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

