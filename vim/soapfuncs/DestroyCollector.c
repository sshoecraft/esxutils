
#include "DestroyCollector.h"

static int put_dc(struct soap *soap, void *arg) {
	struct ManagedObjectReference *obj = arg;
	struct mydesc put_dc_desc[] = {
		{ "_this", &obj, put_ManagedObjectReference, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"DestroyCollector","xmlns",MYNS,put_dc_desc);
}

static int get_dc(struct soap *soap, void *arg) {
	if (soap_element_begin_in(soap, "DestroyCollectorResponse", 1, NULL) || soap_element_end_in(soap, "DestroyCollectorResponse"))
		return 1;
	return SOAP_OK;
}

SOAPFUNCO(DestroyCollector,put_dc,struct ManagedObjectReference *,get_dc);
