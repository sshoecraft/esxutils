
#include "QueryPerfProviderSummary.h"

static int put_qpp(struct soap *soap, void *arg) {
	struct QueryPerfProviderSummaryReq *req = arg;

	if (soap_element(soap,"QueryPerfProviderSummary",0,0) ||
			soap_attribute(soap, "xmlns", MYNS) ||
			soap_element_start_end_out(soap, 0) ||
			send_MOR(soap,"_this",req->perf_obj) ||
			send_MOR(soap,"entity",req->host_obj) ||
			soap_element_end_out(soap,"QueryPerfProviderSummary")) {
		return 1;
	}

	return 0;
}

#if 0
<QueryPerfProviderSummaryResponse xmlns="urn:vim25">
	<returnval>
		<entity type="HostSystem">host-321</entity>
		<currentSupported>true</currentSupported>
		<summarySupported>true</summarySupported>
		<refreshRate>20</refreshRate>
	</returnval>
</QueryPerfProviderSummaryResponse>
#endif
static int get_qpp(struct soap *soap, void *arg) {
	struct QueryPerfProviderSummaryResponse *info = arg;
	struct mydesc qpp_desc[] = {
		{ "entity", &info->entity, get_ManagedObjectReference, 1 },
#if 0
		{ "currentSupported", &info->currentSupported, get_bool, 1 },
		{ "summarySupported", &info->summarySupported, get_bool, 1 },
		{ "refreshRate", &info->refreshRate, get_int, 1 },
#else
		{ "currentSupported", &info->currentSupported, get_char, 1 },
		{ "summarySupported", &info->summarySupported, get_char, 1 },
		{ "refreshRate", &info->refreshRate, get_char, 1 },
#endif
		{ 0,0,0,0 }
	};

	/* must have <QueryPerfProviderSummaryResponse> */
	if (soap_element_begin_in(soap, "QueryPerfProviderSummaryResponse", 0, 0)) {
		dprintf("soap_element_begin_in(QueryPerfProviderSummaryResponse) failed\n");
		goto get_qpp_error;
	}
	dprintf("got <QueryPerfProviderSummaryResponse>\n");

	if (soap_recv_desc(soap, "returnval", qpp_desc))
		goto get_qpp_error;

	/* must have </QueryPerfProviderSummaryResponse> */
	if (soap_element_end_in(soap, "QueryPerfProviderSummaryResponse")) {
		dprintf("soap_element_end_in(QueryPerfProviderSummaryResponse) failed\n");
		return 1;
	}
	dprintf("got </QueryPerfProviderSummaryResponse>\n");

	return 0;

get_qpp_error:
	soap_element_end_in(soap, "QueryAvailablePerfMetricResponse");
	return 1;
}

SOAPFUNCIO(QueryPerfProviderSummary,put_qpp,struct QueryPerfProviderSummaryReq *,get_qpp,struct QueryPerfProviderSummaryResponse *);
