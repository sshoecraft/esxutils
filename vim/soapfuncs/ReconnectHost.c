
#include "ReconnectHost.h"

static int put_rh(struct soap *soap, void *arg) {
	struct ReconnectHostRequest *req = arg;
	struct mydesc put_rh_desc[] = {
		{ "_this", &req->host, put_ManagedObjectReference, 0 },
		{ "cnxSpec", &req->cnxSpec, put_HostConnectSpec, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"ReconnectHost_Task","xmlns",MYNS,put_rh_desc);
}

static int get_rh(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_rh_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "ReconnectHost_TaskResponse", get_rh_desc);
}

SOAPFUNCIO(ReconnectHost,put_rh,struct ReconnectHostRequest *,get_rh,struct ManagedObjectReference **);
