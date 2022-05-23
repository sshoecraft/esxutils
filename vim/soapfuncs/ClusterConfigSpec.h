
#ifndef __VIM_CLUSTERCONFIG_H
#define __VIM_CLUSTERCONFIG_H

#include "soapfuncs.h"
#include "list.h"

struct ClusterDasVmSettings {
	char *isolationResponse;
	char *restartPriority;
};

struct ClusterDasConfigInfo {
	bool admissionControlEnabled;
//	char *admissionControlEnabled;
	struct ClusterDasVmSettings *defaultVmSettings;
	bool enabled;
//	int failoverLevel;
	char *failoverLevel;
	list option;
};

struct ClusterDasVmConfigInfo {
	struct ManagedObjectReference *key;
	bool powerOffOnIsolation;
	char *restartPriority;
};

struct ClusterDrsConfigInfo {
	struct DrsBehavior *defaultVmBehavior;
	bool enabled;
	list option;
//	int vmotionRate;
	char *vmotionRate;
};

struct ClusterDrsVmConfigInfo {
	char *behavior;
	bool enabled;
	struct ManagedObjectReference *key;
};

struct ClusterRuleInfo {
	bool enabled;
//	int key;
	char *key;
	char *name;
	char *status;
};

struct ClusterConfigSpec {
	struct ClusterDasConfigInfo *dasConfig; /* Changes to the configuration of VMware HA.  */
//	list dasVmConfigSpec;			/* List of type ClusterDasVmConfigInfo */
//	struct ClusterDpmConfigInfo *dpmConfig;	/* Changes to VMware DPM service. */
//	list dpmHostConfigSpec;			/* List of type ClusterDpmHostConfigSpec */
	struct ClusterDrsConfigInfo *drsConfig;	/* Changes to the configuration of the VMware DRS service. */
//	list drsVmConfigSpec;			/* List of type ClusterDrsVmConfigSpec */
//	list rulesSpec;				/* List of type ClusterRuleSpec */
};

int put_ClusterConfigSpec(struct soap *soap, char *tag, void **ptr, int req);

#endif
