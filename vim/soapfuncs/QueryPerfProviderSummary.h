
#ifndef __VIM_QueryPerfProviderSummary_H
#define __VIM_QueryPerfProviderSummary_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct QueryPerfProviderSummaryReq {
	struct ManagedObjectReference *perf_obj;
	struct ManagedObjectReference *host_obj;
};

#if 0
struct QueryPerfProviderSummaryResponse {
	bool currentSupported;
	struct ManagedObjectReference *entity;
	int refreshRate;
	bool summarySupported;
};
#else
struct QueryPerfProviderSummaryResponse {
	struct ManagedObjectReference *entity;
	char *currentSupported;
	char *summarySupported;
	char *refreshRate;
};
#endif

SOAPFUNCDEFIO(QueryPerfProviderSummary,struct QueryPerfProviderSummaryReq *,struct QueryPerfProviderSummaryResponse *);

#endif
