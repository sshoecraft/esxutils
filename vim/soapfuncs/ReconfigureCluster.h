
#ifndef __VIM_ReconfigureCluster_H
#define __VIM_ReconfigureCluster_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "ClusterConfigSpec.h"

struct ReconfigureClusterRequest {
	struct ManagedObjectReference *entity;
	struct ClusterConfigSpec *spec;
	bool modify;
};

SOAPFUNCDEFIO(ReconfigureCluster,struct ReconfigureClusterRequest *,struct ManagedObjectReference **);

#endif
