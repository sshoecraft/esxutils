
#ifndef __VIM_DisconnectHost_H
#define __VIM_DisconnectHost_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct DisconnectHostRequest {
	struct ManagedObjectReference *host;
};

SOAPFUNCDEFIO(DisconnectHost,struct DisconnectHostRequest *,struct ManagedObjectReference **);

#endif
