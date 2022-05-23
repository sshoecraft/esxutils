
#include "esxpost.h"
#include <ctype.h>

char *strcpylc(char *dest, char *src, int dlen) {
	register char *dptr, *sptr;

	dptr = dest;
	sptr = src;
	while(dlen--) *dptr++ = tolower(*sptr++);
	*dptr = 0;

	return dest;
}

char *strcpyuc(char *dest, char *src, int dlen) {
	register char *dptr, *sptr;

	dptr = dest;
	sptr = src;
	while(dlen--) *dptr++ = toupper(*sptr++);
	*dptr = 0;

	return dest;
}

