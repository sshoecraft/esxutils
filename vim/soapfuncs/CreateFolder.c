
#include "CreateFolder.h"

static int put_cd(struct soap *soap, void *arg) {
	struct CreateFolderRequest *req = arg;
	struct mydesc put_rh_desc[] = {
		{ "_this", &req->folder, put_ManagedObjectReference, 0 },
		{ "name", &req->name, put_char, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"CreateFolder","xmlns",MYNS,put_rh_desc);
}

static int get_cd(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_cd_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "CreateFolderResponse", get_cd_desc);
}

SOAPFUNCIO(CreateFolder,put_cd,struct CreateFolderRequest *,get_cd,struct ManagedObjectReference **);
