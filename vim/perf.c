
#include "ivim.h"
#include "ServiceContent.h"
#include "QueryPerfProviderSummary.h"
#include "RetrieveProperties.h"
#include "QueryAvailablePerfMetric.h"
#include "QueryPerf.h"

#define CHECK_SUPPORT 0

#if CHECK_SUPPORT
static int vim_checkperfsupport(struct vim_session *s, struct ManagedObjectReference *mor) {
	struct QueryPerfProviderSummaryReq req;
	struct QueryPerfProviderSummaryResponse resp;

	/* Check if the host even supports it */
	req.perf_obj = s->sc->perfManager;
	req.host_obj = mor;
	if (QueryPerfProviderSummary(s->soap, s->endpoint, &req, &resp))
		return 0;

	printf("vim_checkperfsupport: res.currentSupported: %s\n", resp.currentSupported);
	return (strcmp(resp.currentSupported,"true") == 0);
}
#endif

int vim_getallperfmetrics(struct vim_session *s) {
	list results;
	int count;
	struct returnval *ret;
	struct propSet *set, *set2;
	char *p, *none;
	struct vim_perfmetric metric;

	results = vim_getinfo(s, s->sc->perfManager->type, 0, 1, s->sc->perfManager);
	count = list_count(results);
	dprintf("count: %d\n", count);
	if (count < 1) {
		dprintf("error getting perf counters\n");
		return 1;
	}
	none = malloc(4);
	s->metrics = list_create();
	ret = results->first->item;
	set = get_result(ret->nodes,"perfCounter");
	list_reset(set->nodes);
	while((set2 = list_get_next(set->nodes)) != 0) {
		p = get_result_value(set2->nodes, "key");
		if (!p) continue;
//		dprintf("value: %s\n", p);
		metric.id = atoi(p);
		metric.name = get_result_value(set2->nodes,"nameInfo.key");
		if (!metric.name) metric.name = none;
		metric.group = get_result_value(set2->nodes,"groupInfo.key");
		if (!metric.group) metric.group = none;
		metric.unit = get_result_value(set2->nodes,"unitInfo.key");
		if (!metric.unit) metric.unit = none;
		metric.type = get_result_value(set2->nodes,"rollupType");
		if (!metric.type) metric.type = none;

//		dump_metric("all",&metric);
		list_add(s->metrics,&metric,sizeof(metric));
	}

	return 0;
}

#ifdef GET_AVAIL
static list _get_avail(struct vim_session *s, struct ManagedObjectReference *mor, char *startTime) {
	struct QueryAvailablePerfMetricReq req;
	list results;
#ifdef CACHE_AP
	struct vim_apmetric *ap,newap;

	dprintf("mor->type: %s\n", mor->type);

	if (!s->apmetrics) s->apmetrics = list_create();
	list_reset(s->apmetrics);
	while((ap = list_get_next(s->apmetrics)) != 0) {
		dprintf("ap->type: %s\n", ap->type);
		if (strcmp(ap->type,mor->type) == 0) {
			dprintf("found it!\n");
			return ap->results;
		}
	}
#endif

	/* Get list of available metrics */
	memset(&req,0,sizeof(req));
	req.perfManager = s->sc->perfManager;
	req.entity = mor;
	req.beginTime = startTime;
	results = list_create();
	if (QueryAvailablePerfMetric(s->soap, s->endpoint, &req, results)) {
		dprintf("error calling QueryAvailablePerfMetric\n");
		return 0;
	}
#ifdef CACHE_AP
	newap.type = malloc(strlen(mor->type)+1);
	strcpy(newap.type,mor->type);
	newap.results = results;
	list_add(s->apmetrics,&newap,sizeof(newap));
#endif

	return results;
}

static int _chkid(list spec, char *newid) {
	struct counterInfo *counter;

	list_reset(spec);
	while((counter = list_get_next(spec)) != 0) {
		if (strcmp(counter->id,newid) == 0) return 1;
	}
	return 0;
}
#endif

/* How many values to keep a history of for averaging */
#define NUM_HIST 5

/* combine the data with the timestamps */
static struct vim_perfdata *_get_data(char **ts, int *values, int count) {
	struct vim_perfdata *data;
	int i;
#if NUM_HIST
	int hist[NUM_HIST],j,tot,avg;
#endif

	data = malloc(sizeof(*data) * count);
	if (!data) {
		perror("_get_data: malloc vim_perfdata");
		return 0;
	}
#if NUM_HIST
	for(i=0; i < NUM_HIST; i++) hist[i] = 0;
#endif
	for(i=0; i < count; i++) {
		data[i].timeStamp = ts[i];
		data[i].value = values[i];
#if NUM_HIST
		avg = 0;
		if (data[i].value == -1) {
			tot = 0;
			for(j=0; j < NUM_HIST; j++) tot += hist[j];
			data[i].value = tot/NUM_HIST;
			avg = 1;
		}
		for(j=1; j < NUM_HIST; j++) hist[j-1] = hist[j];
		hist[NUM_HIST-1] = data[i].value;
//		for(j=0; j < NUM_HIST; j++) dprintf("hist[%d]: %d\n", j, hist[j]);
//		dprintf("ts: %s, value: %s%d\n", data[i].timeStamp, (avg ? "*" : ""), data[i].value);
#else
//		dprintf("ts: %s, value: %d\n", data[i].timeStamp, data[i].value);
#endif
	}

	return data;
}

int vim_getperfmetrics(struct vim_session *s, struct ManagedObjectReference *mor, char *startTime, struct vim_perfmetric *metrics, int chkavail) {
	struct vim_perfmetric *m1,*m2;
	int have_all,count,i;
	struct perfInfo info;
	struct perfData *data;
	struct sampleInfo *sample;
	struct PerfQuerySpec spec;
//	struct returnval *ret;
	struct QueryPerfProviderSummaryReq req;
	struct QueryPerfProviderSummaryResponse resp;
	char **ts;
//	list results;
#ifdef GET_AVAIL
	int id;
#endif
	volatile int intervalId;
	char temp[16];
	struct counterInfo nc;
	struct counterInfo *counter;

	intervalId = 0;
	req.perf_obj = s->sc->perfManager;
	req.host_obj = mor;
	if (QueryPerfProviderSummary(s->soap, s->endpoint, &req, &resp) == 0) {
		dprintf("resp.summarySupported: %p\n", resp.summarySupported);
		if (resp.summarySupported) dprintf("resp.summarySupported: %s\n", resp.summarySupported);
		if (resp.summarySupported && strcmp(resp.summarySupported,"true") == 0) intervalId = 300;
		if (resp.summarySupported && strcmp(resp.summarySupported,"true") != 0) {
			printf("ERROR: summarySupported is set to False, unable to collect metrics!\n");
			return -1;
		}
		dprintf("refresh: %s\n", resp.refreshRate);
	}
	if (resp.refreshRate) intervalId = atoi(resp.refreshRate);

//	if (!vim_checkperfsupport(s,mor)) return 1;
	if (!s->metrics) { if (vim_getallperfmetrics(s)) return 1; }

	/* First, fill the Ids in the metrics they want */
	list_reset(s->metrics);
	while((m1 = list_get_next(s->metrics)) != 0) {
//		dump_metric("m1",m1);
		have_all = 1;
		for(m2 = metrics; m2->name; m2++) {
			if (!m2->id) have_all = 0;
			else continue;
//			dump_metric("m2",m2);
			if (strcmp(m1->name,m2->name) != 0) continue;
			if (m2->group && strcmp(m1->group,m2->group) != 0) continue;
			if (m2->unit && strcmp(m1->unit,m2->unit) != 0) continue;
			if (m2->type && strcmp(m1->type,m2->type) != 0) continue;
			m2->id = m1->id;
			dprintf("*** FOUND IT ***\n");
			dump_metric("m2",m2);
			break;
		}
//		dprintf("have_all: %d\n", have_all);
		if (have_all) break;
	}

	/* Setup query spec */
	memset(&spec,0,sizeof(spec));
	spec.perf_obj = s->sc->perfManager;
	spec.entity = mor;
	spec.startTime = startTime;
	dprintf("--> intervalId: %d\n", intervalId);
	spec.intervalId = intervalId;
	spec.metricId = list_create();

#ifdef GET_AVAIL
	if (chkavail) {
	/* Get the available metrics */
	results = _get_avail(s,mor,startTime);
	dprintf("count: %d\n", list_count(results));

	/* Check list for the ones we want */
	list_reset(results);
	while(( counter = list_get_next(results)) != 0) {
//		dprintf("id: %s, inst: %s\n", counter->id, counter->instance);
		id = atoi(counter->id);
		for(m2 = metrics; m2->name; m2++) {
			if (!m2->id) continue;
			if (m2->id == id && _chkid(spec.metricId, counter->id) == 0) {
				counter->instance = "";
//				dprintf("adding: id: %s\n", counter->id);
				list_add(spec.metricId, counter, sizeof(*counter));
			}
		}
	}
	} else {
#else
	for(m2 = metrics; m2->name; m2++) {
		if (!m2->id) continue;
		sprintf(temp,"%d",m2->id);
		nc.id = malloc(strlen(temp)+1);
		strcpy(nc.id,temp);
		nc.instance = "";
		list_add(spec.metricId, &nc, sizeof(nc));
	}
#endif
//	}

	if (!list_count(spec.metricId)) {
		dprintf("no metricIds!\n");
		return 1;
	}
	list_reset(spec.metricId);
	while((counter = list_get_next(spec.metricId)) != 0) {
		dprintf("metricId: %s\n", counter->id);
	}

	if (QueryPerf(s->soap, s->endpoint, &spec, &info)) {
		printf("error calling QueryPerf for object %s:%s\n", mor->type, mor->value);
		return 1;
	}
	list_destroy(spec.metricId);

	count = list_count(info.samples);
	dprintf("sample count: %d\n", count);
//	2010-05-10T16:30:00Z
	ts = malloc(count * 32);
	i = 0;
	list_reset(info.samples);
	while((sample = list_get_next(info.samples)) != 0) {
		ts[i] = sample->timestamp;
		if (ts[i][10] == 'T') ts[i][10] = ' ';
		if (ts[i][19] == 'Z') ts[i][19] = 0;
//		dprintf("ts[%d]: %s\n", i, ts[i]);
		i++;
	}
//	dprintf("i: %d\n", i);
	list_destroy(info.samples);

	list_reset(info.data);
	while((data = list_get_next(info.data)) != 0) {
		have_all = 1;
		dprintf("data: counterId: %d, values: %p\n", data->counterId, data->values);
		for(m2 = metrics; m2->name; m2++) {
			if (!m2->data) have_all = 0;
			else continue;
			dprintf("m2->id: %d\n", m2->id);
			if (m2->id == data->counterId) {
				m2->data = _get_data(ts,data->values,count);
				if (!m2->data) {
					have_all = 1;
					break;
				}
				m2->count = count;
				break;
			}
		}
		dprintf("have_all: %d\n", have_all);
		if (have_all) break;
	}

	free(ts);
	list_reset(info.data);
	while((data = list_get_next(info.data)) != 0) free(data->values);
	list_destroy(info.data);
	return 0;
}
