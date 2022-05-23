
#ifndef __VIM_CreateDatacenter_H
#define __VIM_CreateDatacenter_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct CreateDatacenterRequest {
	struct ManagedObjectReference *folder;
	char *name;
};

SOAPFUNCDEFIO(CreateDatacenter,struct CreateDatacenterRequest *,struct ManagedObjectReference **);

#endif
