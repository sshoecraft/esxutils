
#include "Service.h"

static int put_start_req(struct soap *soap, void *arg) {
	struct ServiceRequest *req = arg;
	struct mydesc put_req_desc[] = {
		{ "_this", &req->serviceManager, put_ManagedObjectReference, 1 },
		{ "id", &req->id, put_char, 1 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"StartService","xmlns",MYNS,put_req_desc);
}

static int put_stop_req(struct soap *soap, void *arg) {
	struct ServiceRequest *req = arg;
	struct mydesc put_req_desc[] = {
		{ "_this", &req->serviceManager, put_ManagedObjectReference, 1 },
		{ "id", &req->id, put_char, 1 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"StopService","xmlns",MYNS,put_req_desc);
}

static int get_resp(struct soap *soap, void *arg) {
//	struct ManagedObjectReference **obj = arg;
	struct mydesc get_resp_desc[] = {
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "StartServiceResponse", get_resp_desc);
}

SOAPFUNCO(StartService,put_start_req,struct ServiceRequest *,get_resp);
SOAPFUNCO(StopService,put_stop_req,struct ServiceRequest *,get_resp);
