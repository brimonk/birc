#ifndef FILE_IO
#define FILE_IO

#include <stdarg.h>

enum {
	FIO_MSG,
	FIO_WRN,
	FIO_ERR,
	FIO_VER,
	FIO_NON
};

void fio_setfp(FILE *fp);
void fio_closefp();
int fio_printf(char *file, int line, int level, char *fmt, ...);

#define FIO_PRINTF(level, fmt, ...) \
	fio_printf(__FILE__, __LINE__, (level), (fmt), ##__VA_ARGS__)

#endif
