
#ifndef __VIM_AboutInfo_H
#define __VIM_AboutInfo_H

#include "soapfuncs.h"

struct AboutInfo {
	char *name;
	char *fullName;
	char *vendor;
	char *version;
	char *build;
	char *localeVersion;
	char *localeBuild;
	char *osType;
	char *productLineId;
	char *apiType;
	char *apiVersion;
	char *instanceUuid;
	char *licenseProductName;
	char *licenseProductVersion;
};

struct AboutInfo *new_AboutInfo(struct soap *soap);
int get_AboutInfo(struct soap *soap, char *tag, void **ptr, int req);

#endif
