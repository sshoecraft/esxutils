
#include "ivim.h"
#include <stdarg.h>
#include <stdio.h>

char _vim_errorstr[1024];

void vim_error(char *format,...) {
	char msg[4096], *p;
	va_list ap;

	va_start(ap,format);
	p = msg;
	p += vsprintf(p,format,ap);
	p += sprintf(p,": %s", _vim_errorstr);
	va_end(ap);

	fprintf(stderr,"%s\n", msg);
}
