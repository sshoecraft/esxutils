
#ifndef __VIM_HostConfigSummary_H
#define __VIM_HostConfigSummary_H

#include "soapfuncs.h"

struct HostConfigSummary {
	char *name;
//	int port;
	char *port;
	struct AboutInfo *product;
	bool vmotionEnabled;
};

#endif
