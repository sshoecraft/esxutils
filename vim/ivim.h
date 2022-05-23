
#include "vim.h"

#ifndef dprintf
#ifdef DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(format, args...) /* noop */
#endif
#endif

//#define _vim_seterr(msg,args...) snprintf(_vim_errorstr,sizeof(_vim_errorstr),msg,## args)

#if 0
#define CHECKSC(s)  \
	if (!s->sc || !s->sc->sessionManager) {  \
		_vim_seterr("vim_login: invalid serviceContent");  \
		return 1;  \
	}
#endif

extern char *empty_str;
