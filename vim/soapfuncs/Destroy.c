
#include "Destroy.h"

static int put_req(struct soap *soap, void *arg) {
	struct ManagedObjectReference *mo_ref = arg;
	struct mydesc put_req_desc[] = {
		{ "_this", &mo_ref, put_ManagedObjectReference, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"Destroy_Task","xmlns",MYNS,put_req_desc);
}

static int get_resp(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_resp_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "Destroy_TaskResponse", get_resp_desc);
}

SOAPFUNCIO(Destroy,put_req,struct ManagedObjectReference *,get_resp,struct ManagedObjectReference **);
