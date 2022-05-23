
#include "CreateCluster.h"

static int put_cc(struct soap *soap, void *arg) {
	struct CreateClusterRequest *req = arg;
	if (req) {
		struct mydesc put_req_desc[] = {
			{ "_this", &req->folder, put_ManagedObjectReference, 1 },
			{ "name", &req->name, put_char, 1 },
			{ "spec", &req->spec, put_ClusterConfigSpec, 1 },
			{ 0,0,0,0 }
		};

		return soap_send_desc(soap,"CreateCluster","xmlns",MYNS,put_req_desc);
	}
	return 1;
}

static int get_cc(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_cc_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 1 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "CreateClusterResponse", get_cc_desc);
}

SOAPFUNCIO(CreateCluster,put_cc,struct CreateClusterRequest *,get_cc,struct ManagedObjectReference **);
