
#include "CreateCollectorForTasks.h"

static int put_cct(struct soap *soap, void *arg) {
	struct CreateCollectorForTasksRequest *req = arg;
	struct mydesc put_cct_desc[] = {
		{ "_this", &req->taskManager, put_ManagedObjectReference, 0 },
		{ "filter", &req->filter, put_TaskFilterSpec, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"CreateCollectorForTasks","xmlns",MYNS,put_cct_desc);
}

static int get_cct(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_exitmm_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "CreateCollectorForTasksResponse", get_exitmm_desc);
}

SOAPFUNCIO(CreateCollectorForTasks,put_cct,struct CreateCollectorForTasksRequest *,get_cct,struct ManagedObjectReference **);
