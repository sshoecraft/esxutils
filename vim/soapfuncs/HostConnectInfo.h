
#ifndef __VIM_HostConnectInfo_H
#define __VIM_HostConnectInfo_H

#include "soapfuncs.h"
#include "HostListSummary.h"

struct HostDatastoreConnectInfo {
	char dummy;
};

struct HostConnectInfoNetworkInfo {
	char dummy;
};

struct VirtualMachineSummary {
	char dummy;
};

struct HostConnectInfo {
	bool clusterSupported;
	list datastore;
	struct HostListSummary *host;
	list network;
	char *serverIp;
	bool vimAccountNameRequired;
	list vm;
};

int get_HostConnectInfo(struct soap *soap, char *tag, void **ptr, int req);

#endif
