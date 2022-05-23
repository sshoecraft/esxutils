
#ifndef __VIM_HostListSummary_H
#define __VIM_HostListSummary_H

#include "soapfuncs.h"
#include "HostConfigSummary.h"
#include "list.h"
#include "HostHardwareSummary.h"
#include "ManagedObjectReference.h"
#include "HostListSummaryQuickStats.h"
#include "HostRuntimeInfo.h"

struct HostListSummary {
	struct HostConfigSummary *config;
	list customValue;
	struct HostHardwareSummary *hardware;
	struct ManagedObjectReference *host;
	char *overallStatus;
	struct HostListSummaryQuickStats *quickStats;
	bool rebootRequired;
	struct HostRuntimeInfo *runtime;
};

int get_HostListSummary(struct soap *soap, char *tag, void **ptr, int req);

#endif
