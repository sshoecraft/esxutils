
#include "ReconfigureCluster.h"

static int put_req(struct soap *soap, void *arg) {
	struct ReconfigureClusterRequest *req = arg;
	if (req) {
		struct mydesc put_req_desc[] = {
			{ "_this", &req->entity, put_ManagedObjectReference, 1 },
			{ "spec", &req->spec, put_ClusterConfigSpec, 1 },
			{ "modify", &req->modify, put_bool, 0 },
			{ 0,0,0,0 }
		};

		return soap_send_desc(soap,"ReconfigureCluster_Task","xmlns",MYNS,put_req_desc);
	}
	return 1;
}

static int get_resp(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_resp_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 1 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "ReconfigureCluster_TaskResponse", get_resp_desc);
}

SOAPFUNCIO(ReconfigureCluster,put_req,struct ReconfigureClusterRequest *,get_resp,struct ManagedObjectReference **);
