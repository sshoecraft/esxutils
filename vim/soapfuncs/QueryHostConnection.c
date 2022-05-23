
#include "QueryHostConnection.h"

static int put_req(struct soap *soap, void *arg) {
	struct ManagedObjectReference *obj = arg;
	struct mydesc put_req_desc[] = {
		{ "_this", &obj, put_ManagedObjectReference, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"QueryHostConnectionInfo","xmlns",MYNS,put_req_desc);
}

static int get_res(struct soap *soap, void *arg) {
	struct HostConnectInfo *info = arg;
	struct mydesc get_res_desc[] = {
//		{ "clusterSupported", &info->clusterSupported, get_bool, 0 },
//		{ "datastore", &info->datastore, get_HostDatastoreConnectInfo, 0 },
		{ "host", &info->host, get_HostListSummary, 0 },
//		{ "network", &info->network, get_HostConnectInfoNetworkInfo, 0 },
		{ "serverIp", &info->serverIp, get_char, 0 },
//		{ "vimAccountNameRequired", &info->vimAccountNameRequired, get_bool, 0 },
//		{ "vm", &info->vm, get_VirtualMachineSummary, 0 },
		{ 0,0,0,0 }
	};

	if (soap_element_begin_in(soap, "QueryHostConnectionInfoResponse", 0, 0)
		|| soap_element_begin_in(soap, "returnval", 0, 0)
		|| soap_recv_desc(soap, 0, get_res_desc)
		|| soap_element_end_in(soap, "returnval")
		|| soap_element_end_in(soap, "QueryHostConnectionInfoResponse"))
		return 1;

	return 0;
}

SOAPFUNCIO(QueryHostConnectionInfo,put_req,struct ManagedObjectReference *,get_res,struct HostConnectInfo *);
