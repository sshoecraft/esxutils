
#ifndef __VIM_EnterMaintenanceMode_H
#define __VIM_EnterMaintenanceMode_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct EnterMaintenanceModeRequest {
	struct ManagedObjectReference *obj;
	int timeout;
	bool evacuatePoweredOffVms;
};

SOAPFUNCDEFIO(EnterMaintenanceMode,struct EnterMaintenanceModeRequest *,struct ManagedObjectReference **)

#endif
