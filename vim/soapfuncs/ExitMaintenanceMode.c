
#include "ExitMaintenanceMode.h"

static int put_exitmm(struct soap *soap, void *arg) {
	struct ExitMaintenanceModeRequest *req = arg;
	struct mydesc put_exitmm_desc[] = {
		{ "_this", &req->obj, put_ManagedObjectReference, 0 },
		{ "timeout", &req->timeout, put_int, 0 },
		{ 0,0,0 }
	};

	return soap_send_desc(soap, "ExitMaintenanceMode_Task", "xmlns", MYNS, put_exitmm_desc);
}

static int get_exitmm(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_exitmm_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

#if 0
        if (soap_element_begin_in(soap, "LoginResponse", 0, 0)
                || soap_element_begin_in(soap, "returnval", 0, 0)
                || soap_recv_desc(soap, 0, get_exitmm_desc)
                || soap_element_end_in(soap, "returnval")
                || soap_element_end_in(soap, "LoginResponse"))
                return 1;

	return 0;
#endif
	return soap_recv_desc(soap, "ExitMaintenanceMode_TaskResponse", get_exitmm_desc);
}

SOAPFUNCIO(ExitMaintenanceMode,put_exitmm,struct ExitMaintenanceModeRequest *,get_exitmm,struct ManagedObjectReference **);
