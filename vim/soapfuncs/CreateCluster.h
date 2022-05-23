
#ifndef __VIM_CreateCluster_H
#define __VIM_CreateCluster_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "ClusterConfigSpec.h"

struct CreateClusterRequest {
	struct ManagedObjectReference *folder;
	char *name;
	struct ClusterConfigSpec *spec;
};

SOAPFUNCDEFIO(CreateCluster,struct CreateClusterRequest *,struct ManagedObjectReference **);

#endif
