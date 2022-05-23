
#ifndef __VIM_ExitMaintenanceMode_H
#define __VIM_ExitMaintenanceMode_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct ExitMaintenanceModeRequest {
	struct ManagedObjectReference *obj;
	int timeout;
};

SOAPFUNCDEFIO(ExitMaintenanceMode,struct ExitMaintenanceModeRequest *,struct ManagedObjectReference **);

#endif
