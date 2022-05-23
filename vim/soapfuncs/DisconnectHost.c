
#include "DisconnectHost.h"

static int put_rh(struct soap *soap, void *arg) {
	struct DisconnectHostRequest *req = arg;
	struct mydesc put_rh_desc[] = {
		{ "_this", &req->host, put_ManagedObjectReference, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"DisconnectHost_Task","xmlns",MYNS,put_rh_desc);
}

static int get_rh(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_rh_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "DisconnectHost_TaskResponse", get_rh_desc);
}

SOAPFUNCIO(DisconnectHost,put_rh,struct DisconnectHostRequest *,get_rh,struct ManagedObjectReference **);
