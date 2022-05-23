
#include "QueryPerf.h"

#if 0
	endTime* xsd:dateTime 
	entity ManagedObjectReference 
	format* xsd:string 
	intervalId* xsd:int 
	maxSample* xsd:int 
	metricId* PerfMetricId[] 
	startTime* xsd:dateTime 
#endif

static int put_metricId(struct soap *soap, char *tag, void **ptr, int req) {
	list metricIds = (list) *ptr;
	struct counterInfo *info;

	list_reset(metricIds);
	while((info = list_get_next(metricIds)) != 0) {
//		dprintf("info->id: %p, info->instance: %p\n", info->id, info->instance);
		soap_element_begin_out(soap,"metricId",0,0);
		soap_send_string(soap, "counterId", info->id);
		soap_send_string(soap, "instance", info->instance);
		soap_element_end_out(soap,"metricId");
	}
	return 0;
}

static int put_qp(struct soap *soap, void *arg) {
	struct PerfQuerySpec *spec = arg;
	/* XXX order here is important!!! */
	struct mydesc put_qp_desc[] = {
		{ "entity", &spec->entity, put_ManagedObjectReference, 0 },
		{ "startTime", &spec->startTime, put_char, 0 },
		{ "metricId", &spec->metricId, put_metricId, 0 },
		{ "endTime", &spec->endTime, put_char, 0 },
		{ "format", &spec->format, put_char, 0 },
		{ "intervalId", &spec->intervalId, put_int, 0 },
//		{ "maxSample", &spec->maxSample, put_int, 0 },
		{ 0,0,0,0 }
	};

//	dprintf("putting header...\n");
	if (soap_element(soap,"QueryPerf",0,0) ||
			soap_attribute(soap, "xmlns", MYNS) ||
			soap_element_start_end_out(soap, 0) ||
			send_MOR(soap,"_this",spec->perf_obj))
		return 1;

	soap_send_desc(soap, "querySpec", "xmlns", MYNS, put_qp_desc);
	soap_element_end_out(soap,"QueryPerf");

//	dprintf("put done\n");
	return 0;
}

static int get_qp(struct soap *soap, void *arg) {
	struct perfInfo *info = arg;
	struct perfData data;
	struct sampleInfo sample;
	struct ManagedObjectReference obj;
	struct mydesc qp_sample_desc[] = {
		{ "timestamp", &sample.timestamp, get_char, 1 },
		{ "interval", &sample.interval, get_char, 1 },
		{ 0,0,0,0 }
	};
	struct counterInfo counter;
	struct mydesc qp_counter_desc[] = {
		{ "counterId", &counter.id, get_char, 1 },
		{ "instance", &counter.instance, get_char, 0 },
		{ 0,0,0,0 }
	};
	int sample_count, index, count;
	int *values;
	void *ptr = &obj;
	char *value;

	value = 0;
	info->samples = list_create();
	info->data = list_create();

	dprintf("getting perf...\n");
	/* must have <QueryPerfResponse> */
	if (soap_element_begin_in(soap, "QueryPerfResponse", 0, 0)) {
		dprintf("soap_element_begin_in(QueryPerfResponse) failed\n");
		goto get_qp_error;
	}
	dprintf("got <QueryPerfResponse>\n");

	/* No more tags if no results are found */
	if (soap_peek_element(soap) != SOAP_OK) return soap_element_end_in(soap, "QueryPerfResponse");

	/* must have <returnval> */
	if (soap_element_begin_in(soap, "returnval", 0, 0)) {
		dprintf("soap_element_begin_in(returnval) failed\n");
		goto get_qp_error;
	}
	dprintf("got <returnval>\n");

	/* must have <entity> */
	if (get_ManagedObjectReference(soap, "entity", &ptr, 1)) {
		dprintf("soap_element_begin_in(returnval) failed\n");
		goto get_qp_error;
	}
	dprintf("got <entity>\n");

	/* no CSV support */
	if (soap_peek_element(soap) != SOAP_OK) goto get_qp_error;
	dprintf("tag: %s\n", soap->tag);
	if (strcmp(soap->tag,"sampleInfoCSV") == 0) return 1;

	/* Samples are in <interval>,<timestamp> pairs */
	info->samples = list_create();
	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"sampleInfo") == 0) {
		memset(&sample,0,sizeof(sample));
		if (soap_recv_desc(soap, "sampleInfo", qp_sample_desc))
			goto get_qp_error;
		list_add(info->samples,&sample,sizeof(sample));
	}

	sample_count = list_count(info->samples);
	dprintf("sample count: %d\n", sample_count);

	/* Get the values */
	info->data = list_create();
	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"value") == 0) {
		soap_element_begin_in(soap, "value", 0, 0);

		dprintf("getting id...\n");
		if (soap_recv_desc(soap, "id", qp_counter_desc))
			goto get_qp_error;

		dprintf("getting values...\n");
		values = malloc(sizeof(int *) * sample_count);
		count = 0;
		for(index = 0; index < sample_count; index++) {
			if (soap_recv_string(soap, "value", &value, 1))
				goto get_qp_error;
//			dprintf("value: %s\n", value);
			values[index] = atoi(value);
			count++;
		}
		soap_element_end_in(soap, "value");
		dprintf("value count: %d\n", count);
		if (count != sample_count) {
			dprintf("error: value count != sample_count\n");
			goto get_qp_error;
		}

		data.counterId = atoi(counter.id);
		data.instance = counter.instance;
		data.values = values;
		list_add(info->data,&data,sizeof(data));
	}

	if (soap_element_end_in(soap, "returnval")) {
		dprintf("soap_element_end_in(returnval) failed\n");
		return 1;
	}
	dprintf("got </returnval>\n");

	/* must have </QueryPerfResponse> */
	if (soap_element_end_in(soap, "QueryPerfResponse")) {
		dprintf("soap_element_end_in(QueryPerfResponse) failed\n");
		return 1;
	}
	dprintf("got </QueryPerfResponse>\n");

	return 0;

get_qp_error:
	soap_element_end_in(soap, "QueryPerfResponse");
	return 1;
}

SOAPFUNCIO(QueryPerf,put_qp,struct PerfQuerySpec *,get_qp,struct perfInfo *);
