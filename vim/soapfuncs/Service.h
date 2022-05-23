
#ifndef __VIM_Service_H
#define __VIM_Service_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "list.h"

struct ServiceRequest {
	struct ManagedObjectReference *serviceManager;
	char *id;
};

SOAPFUNCDEFO(StartService,struct ServiceRequest *);
SOAPFUNCDEFO(StopService,struct ServiceRequest *);

#endif
