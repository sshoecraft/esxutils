
#ifndef __VIM_QueryAvailablePerfMetric_H
#define __VIM_QueryAvailablePerfMetric_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "list.h"
#include "counterInfo.h"

struct QueryAvailablePerfMetricReq {
	struct ManagedObjectReference *perfManager;
	struct ManagedObjectReference *entity;
	char *beginTime;
	char *endTime;
	char *intervalId;
};

SOAPFUNCDEFIO(QueryAvailablePerfMetric,struct QueryAvailablePerfMetricReq *,list);

#endif
