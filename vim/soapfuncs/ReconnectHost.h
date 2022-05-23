
#ifndef __VIM_ReconnectHost_H
#define __VIM_ReconnectHost_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "HostConnectSpec.h"

struct ReconnectHostRequest {
	struct ManagedObjectReference *host;
	struct HostConnectSpec *cnxSpec;
};

SOAPFUNCDEFIO(ReconnectHost,struct ReconnectHostRequest *,struct ManagedObjectReference **);

#endif
