
#ifndef __VIM_OptionValue_H
#define __VIM_OptionValue_H

#include "soapfuncs.h"

struct OptionValue {
	char *key;
	char *value;
};

int put_OptionValue(struct soap *soap, char *tag, void **ptr, int req);

#endif
