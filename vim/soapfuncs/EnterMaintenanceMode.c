
#include "EnterMaintenanceMode.h"

static int put_entermm(struct soap *soap, void *arg) {
	struct EnterMaintenanceModeRequest *req = arg;
	struct mydesc put_entermm_desc[] = {
		{ "_this", &req->obj, put_ManagedObjectReference, 0 },
		{ "timeout", &req->timeout, put_int, 0 },
		{ "evacuatePoweredOffVms", &req->evacuatePoweredOffVms, put_bool, 0 },
		{ 0,0,0 }
	};

	return soap_send_desc(soap, "EnterMaintenanceMode_Task", "xmlns", MYNS, put_entermm_desc);
}

static int get_entermm(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_entermm_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "EnterMaintenanceMode_TaskResponse", get_entermm_desc);
}

SOAPFUNCIO(EnterMaintenanceMode,put_entermm,struct EnterMaintenanceModeRequest *,get_entermm,struct ManagedObjectReference **);
