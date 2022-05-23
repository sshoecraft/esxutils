
#include "QueryAvailablePerfMetric.h"


static int put_qap(struct soap *soap, void *arg) {
	struct QueryAvailablePerfMetricReq *req = arg;
	struct mydesc desc[] = {
		{ "_this", &req->perfManager, put_ManagedObjectReference, 0 },
		{ "entity", &req->entity, put_ManagedObjectReference, 0 },
		{ "beginTime", &req->beginTime, put_char, 0 },
		{ "endTime", &req->endTime, put_char, 0 },
		{ "intervalId", &req->intervalId, put_char, 0 },
                { 0,0,0,0 }
        };

	return soap_send_desc(soap,"QueryAvailablePerfMetric","xmlns",MYNS,desc);

#if 0
	if (soap_element(soap,"QueryAvailablePerfMetric",0,0) ||
			soap_attribute(soap, "xmlns", MYNS) ||
			soap_element_start_end_out(soap, 0) ||
			send_MOR(soap,"_this",req->perf_obj) ||
			send_MOR(soap,"entity",req->host_obj) ||
			soap_element_end_out(soap,"QueryAvailablePerfMetric")) {
		return 1;
	}

	return 0;
#endif
}

static int get_qap(struct soap *soap, void *arg) {
	list results = arg;
	struct counterInfo info;
	struct mydesc qap_desc[] = {
		{ "counterId", &info.id, get_char, 1 },
		{ "instance", &info.instance, get_char, 1 },
		{ 0,0,0,0 }
	};

	/* must have <QueryAvailablePerfMetricResponse> */
	if (soap_element_begin_in(soap, "QueryAvailablePerfMetricResponse", 0, 0)) {
		dprintf("soap_element_begin_in(QueryAvailablePerfMetricResponse) failed\n");
		exit(1);
		return 1;
	}
	dprintf("got <QueryAvailablePerfMetricResponse>\n");

	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"returnval") == 0) {
		memset(&info,0,sizeof(info));
		if (soap_recv_desc(soap, "returnval", qap_desc))
			goto get_qap_error;
		list_add(results,&info,sizeof(info));
	}

	/* must have </RetrievePropertiesResponse> */
	if (soap_element_end_in(soap, "QueryAvailablePerfMetricResponse")) {
		dprintf("soap_element_end_in(QueryAvailablePerfMetricResponse) failed\n");
		return 1;
	}
	dprintf("got </QueryAvailablePerfMetricResponse>\n");

	return 0;

get_qap_error:
	soap_element_end_in(soap, "QueryAvailablePerfMetricResponse");
	return 1;
}

SOAPFUNCIO(QueryAvailablePerfMetric,put_qap,struct QueryAvailablePerfMetricReq *,get_qap,list);
