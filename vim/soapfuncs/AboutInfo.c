
#include "AboutInfo.h"

struct AboutInfo *new_AboutInfo(struct soap *soap) {
	struct AboutInfo *info;

	info = soap_malloc(soap, sizeof(struct AboutInfo));
	dprintf("info: %p\n", info);
	memset(info,0,sizeof(struct AboutInfo));
        return info;
}

#if 1
int get_AboutInfo(struct soap *soap, char *tag, void **ptr, int req) {
	struct AboutInfo *info = *ptr;
	register struct mydesc *dp;
	int r;

	dprintf("info: %p\n", info);
	if (!info) {
		*ptr = new_AboutInfo(soap);
		info = *ptr;
	}

	r = 0;
	if (info) {
		struct mydesc about_desc[] = {
			{ "name", &info->name, get_char, 1 },
			{ "fullName", &info->fullName, get_char, 1 },
			{ "vendor", &info->vendor, get_char, 1 },
			{ "version", &info->version, get_char, 1 },
			{ "build", &info->build, get_char, 1 },
			{ "localeVersion", &info->localeVersion, get_char, 0 },
			{ "localeBuild", &info->localeBuild, get_char, 0 },
			{ "osType", &info->osType, get_char, 1 },
			{ "productLineId", &info->productLineId, get_char, 1 },
			{ "apiType", &info->apiType, get_char, 1 },
			{ "apiVersion", &info->apiVersion, get_char, 1 },
			{ "instanceUuid", &info->instanceUuid, get_char, 0 },
			{ "licenseProductName", &info->licenseProductName, get_char, 0 },
			{ "licenseProductVersion", &info->licenseProductVersion, get_char, 0 },
			{ 0,0,0 }
		};
		dprintf("receiving aboutInfo...\n");
		r = soap_recv_desc(soap, tag, about_desc);
		dprintf("r: %d\n", r);
		if (!r) {
			for(dp = about_desc; dp->tag; dp++) {
				char **ptr;
				if (!dp->dest) continue;
				ptr = dp->dest;
				dprintf("tag: %s, dest: %p\n", dp->tag, *ptr);
				if (!*ptr) *ptr = "";
			}
		}
	} else {
		r = 1;
	}
		return r;
}
#else
static int get_about_elements(struct soap *soap, struct AboutInfo *about) {
	struct mydesc about_desc[] = {
		{ "name", &about->name, get_char, 1 },
		{ "fullName", &about->fullName, get_char, 1 },
		{ "vendor", &about->vendor, get_char, 1 },
		{ "version", &about->version, get_char, 1 },
		{ "build", &about->build, get_char, 1 },
		{ "localeVersion", &about->localeVersion, get_char, 0 },
		{ "localeBuild", &about->localeBuild, get_char, 0 },
		{ "osType", &about->osType, get_char, 1 },
		{ "productLineId", &about->productLineId, get_char, 1 },
		{ "apiType", &about->apiType, get_char, 1 },
		{ "apiVersion", &about->apiVersion, get_char, 1 },
		{ "instanceUuid", &about->instanceUuid, get_char, 0 },
		{ "licenseProductName", &about->licenseProductName, get_char, 0 },
		{ "licenseProductVersion", &about->licenseProductVersion, get_char, 0 },
		{ 0,0,0 }
	};

	if (soap_recv_desc(soap, 0, about_desc)) {
		dprintf("soap_recv_desc failed\n");
		return 1;
	}

	return 0;
}

int get_AboutInfo(struct soap *soap, char *tag, void **ptr, int req) {
	struct AboutInfo *about;
//	dprintf("ptr: %p\n", ptr);
	*ptr = soap_malloc(soap, sizeof(struct AboutInfo));
	if (!*ptr) {
		dprintf("malloc error\n");
		return 1;
	}
	about = *ptr;
	if (soap_element_begin_in(soap, tag, 1, NULL)
			|| get_about_elements(soap, about)
			|| soap_element_end_in(soap, tag))
		return 1;

	return 0;
}
#endif
