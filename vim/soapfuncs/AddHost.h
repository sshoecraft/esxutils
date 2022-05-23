
#ifndef __VIM_AddHost_H
#define __VIM_AddHost_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "HostConnectSpec.h"

struct AddHostRequest {
	struct ManagedObjectReference *cluster;
	struct HostConnectSpec *spec;
	bool asConnected;
	struct ManagedObjectReference *resourcePool;
	char *license;
};

SOAPFUNCDEFIO(AddHost,struct AddHostRequest *,struct ManagedObjectReference **);

#endif
