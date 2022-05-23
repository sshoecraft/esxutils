
#ifndef __VIM_HostConnectSpec_H
#define __VIM_HostConnectSpec_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct HostConnectSpec {
	bool force;
	char *hostName;
	char *password;
	int port;
	char *sslThumbprint;
	char *userName;
	char *vimAccountName;
	char *vimAccountPassword;
	struct ManagedObjectReference *vmFolder;
};

int put_HostConnectSpec(struct soap *soap, char *tag, void **ptr, int req);

#endif
