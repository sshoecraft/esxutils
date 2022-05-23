
#include "HostListSummary.h"

int get_HostListSummary(struct soap *soap, char *tag, void **ptr, int req) {
	struct HostListSummary *info = *ptr;
	if (!info) info = *ptr = soap_malloc(soap,sizeof(struct HostListSummary));
	if (info) {
		struct mydesc desc[] = {
//			{ "config", &info->config, get_HostConfigSummary, 0 },
//			{ "customValue", &info->customValue, get_customValue, 0 },
//			{ "hardware", &info->hardware, get_HostHardwareSummary, 0 },
			{ "host", &info->host, get_ManagedObjectReference, 0 },
			{ "overallStatus", &info->overallStatus, get_char, 0 },
//			{ "quickStats", &info->quickStats, get_HostListSummaryQuickStats, 0 },
			{ "rebootRequired", &info->rebootRequired, get_bool, 0 },
//			{ "runtime", &info->runtime, get_HostRuntimeInfo, 0 },
			{ 0,0,0,0 }
		};

		return soap_recv_desc(soap,"HostListSummary",desc);
	}
	return 1;
}
