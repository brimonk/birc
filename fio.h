#ifndef FILE_IO
#define FILE_IO

#include <stdarg.h>

enum {
	FIO_NON,
	FIO_LOG,
	FIO_MSG,
	FIO_WRN,
	FIO_ERR,
	FIO_VER
};

FILE *fio_getstaticfp();
int fio_lines(FILE *fp);
int fio_printf(char *file, int line, int level, char *fmt, ...);
int fio_getline(FILE *fp, char *buf, int buflen, int line);
void fio_closefp();
void fio_setfp(FILE *fp);

#define FIO_PRINTF(level, fmt, ...) \
	fio_printf(__FILE__, __LINE__, (level), (fmt), ##__VA_ARGS__)

#endif
