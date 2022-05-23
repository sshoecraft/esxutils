
#ifndef __VIM_QueryPerf_H
#define __VIM_QueryPerf_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "counterInfo.h"
#include "list.h"

struct PerfQuerySpec {
	struct ManagedObjectReference *perf_obj;
	char *endTime;
	struct ManagedObjectReference *entity;
	char *format;
	int intervalId;
	int maxSample;
	list metricId;			/* list of PerfMetricId */
	char *startTime;
};

struct sampleInfo {
	char *timestamp;
	char *interval;
};

struct perfData {
	int counterId;
	char *instance;
	int *values;
};

struct perfInfo {
	list samples;			/* List of sampleInfo */
	list data;			/* List of perfData */
};

SOAPFUNCDEFIO(QueryPerf,struct PerfQuerySpec *,struct perfInfo *);

#endif
