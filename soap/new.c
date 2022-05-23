
#include "stdsoap2.h"

SOAP_FMAC1 struct soap *SOAP_FMAC2 soap_new(void) {
	struct soap *soap = (struct soap*)malloc(sizeof(struct soap));
	if (soap) {
		soap_init(soap);
		soap_imode(soap, SOAP_IO_DEFAULT);
		soap_omode(soap, SOAP_IO_DEFAULT);
	}
	return soap;
}
