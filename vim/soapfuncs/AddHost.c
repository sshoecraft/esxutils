
#include "AddHost.h"

#if 0
         <complexType name="AddHostRequestType">
            <sequence>
               <element name="_this" type="vim25:ManagedObjectReference" />
               <element name="spec" type="vim25:HostConnectSpec" />
               <element name="asConnected" type="xsd:boolean" />
               <element name="resourcePool" type="vim25:ManagedObjectReference" minOccurs="0" />
               <element name="license" type="xsd:string" minOccurs="0" />
            </sequence>
         </complexType>
#endif

static int put_req(struct soap *soap, void *arg) {
	struct AddHostRequest *req = arg;
	struct mydesc put_req_desc[] = {
		{ "_this", &req->cluster, put_ManagedObjectReference, 0 },
		{ "spec", &req->spec, put_HostConnectSpec, 0 },
		{ "asConnected", &req->asConnected, put_bool, 0 },
		{ "resourcePool", &req->resourcePool, put_ManagedObjectReference, 0 },
		/* XXX only in 4.0 and above */
		{ "license", &req->license, put_char, 0 },
		{ 0,0,0,0 }
	};

//	dprintf("AddHostRequest: _this: %s:%s, asConnected: %d, resourcePool: %s:%s, license: %s\n",
//		req->cluster->type, req->cluster->value, req->asConnected, req->resourcePool->type, req->resourcePool->value, req->license);
	return soap_send_desc(soap,"AddHost_Task","xmlns",MYNS,put_req_desc);
}

static int get_resp(struct soap *soap, void *arg) {
	struct ManagedObjectReference **obj = arg;
	struct mydesc get_resp_desc[] = {
		{ "returnval", obj, get_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_recv_desc(soap, "AddHost_TaskResponse", get_resp_desc);
}

SOAPFUNCIO(AddHost,put_req,struct AddHostRequest *,get_resp,struct ManagedObjectReference **);
