
#ifndef __VIM_DynamicProperty_H
#define __VIM_DynamicProperty_H

#include "soapfuncs.h"
#include "list.h"

struct DynamicProperty {
	char *name;
	char *val;
};

struct DynamicData {
	list dynamicProperty;
	char *dynamicType;
};

int get_DynamicProperty(struct soap *soap, char *tag, void **ptr, int req);

#endif
