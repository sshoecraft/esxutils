
#include "ClusterConfigSpec.h"

static int put_ClusterDasVmSettings(struct soap *soap, char *tag, void **ptr, int req) {
	struct ClusterDasVmSettings *info = *ptr;
	dprintf("ptr: %p, info: %p\n", ptr, info);
	if (info) {
		struct mydesc put_cdms_desc[] = {
			{ "isolationResponse", &info->isolationResponse, put_char, 0 },
			{ "restartPriority", &info->restartPriority, put_char, 0 },
			{ 0,0,0,0 }
		};

		return soap_send_desc(soap,tag,"xsi:type","ClusterDasVmSettings",put_cdms_desc);
	}
	return 0;
}

static int put_ClusterDasConfigInfo(struct soap *soap, char *tag, void **ptr, int req) {
	struct ClusterDasConfigInfo *info = *ptr;
	dprintf("ptr: %p, info: %p\n", ptr, info);
	if (info) {
		struct mydesc put_das_desc[] = {
			{ "enabled", &info->enabled, put_bool, 0 },
			/* XXX failoverLevel is REQUIRED if admissionControlEnabled is specified (true or false) */
			{ "failoverLevel", &info->failoverLevel, put_char, 1 },
			{ "admissionControlEnabled", &info->admissionControlEnabled, put_bool, 0 },
			{ "defaultVmSettings", &info->defaultVmSettings, put_ClusterDasVmSettings, 0 },
			{ "option", &info->option, put_OptionValue, 0 },
			{ 0,0,0,0 }
		};

		return soap_send_desc(soap,tag,"xsi:type","ClusterDasConfigInfo",put_das_desc);
	}
	return 0;
}

static int put_ClusterDrsConfigInfo(struct soap *soap, char *tag, void **ptr, int req) {
	struct ClusterDrsConfigInfo *info = *ptr;
	dprintf("ptr: %p, info: %p\n", ptr, info);
	if (info) {
		struct mydesc put_drs_desc[] = {
			{ "defaultVmBehavior", &info->defaultVmBehavior, put_char, 0 },
			{ "enabled", &info->enabled, put_bool, 0 },
			{ "option", &info->option, put_OptionValue, 0 },
			/* XXX causes CreateCluster to fail */
			{ "vmotionRate", &info->vmotionRate, put_char, 0 },
			{ 0,0,0,0 }
		};

		return soap_send_desc(soap,tag,"xsi:type","ClusterDrsConfigInfo",put_drs_desc);
	}
	return 0;
}

int put_ClusterConfigSpec(struct soap *soap, char *tag, void **ptr, int req) {
	struct ClusterConfigSpec *spec = *ptr;
	dprintf("ptr: %p, spec: %p\n", ptr, spec);
	if (spec) {
		struct mydesc put_ccs_desc[] = {
			{ "dasConfig", &spec->dasConfig, put_ClusterDasConfigInfo, 1 },
			{ "drsConfig", &spec->drsConfig, put_ClusterDrsConfigInfo, 1 },
			{ 0,0,0,0 }
		};

		return soap_send_desc(soap,tag,"xsi:type","ClusterConfigSpec",put_ccs_desc);
	}
	return 0;
}
