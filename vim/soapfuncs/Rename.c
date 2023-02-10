
#include "Rename.h"

static int put_req(struct soap *soap, void *arg) {
	struct RenameRequest *req = arg;
	struct mydesc put_rename_desc[] = {
		{ "_this", &req->obj, put_ManagedObjectReference, 0 },
		{ "newName", &req->newName, put_char, 1 },
		{ 0,0,0,0 }
	};

	dprintf("newname: %s\n", req->newName);
	return soap_send_desc(soap,"Rename_Task","xmlns",MYNS,put_rename_desc);
}

static int get_resp(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_resp_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "Rename_TaskResponse", get_resp_desc);
}

SOAPFUNCIO(Rename,put_req,struct RenameRequest *,get_resp,struct ManagedObjectReference **);
