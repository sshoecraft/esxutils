
#include "Logout.h"

static int put_logout(struct soap *soap, void *arg) {
	struct ManagedObjectReference *obj = arg;
	struct mydesc put_logout_desc[] = {
		{ "_this", &obj, put_ManagedObjectReference, 0 },
		{ 0,0,0 }
	};

	return soap_send_desc(soap, "Logout", "xmlns", MYNS, put_logout_desc);
}

static int get_logout(struct soap *soap, void *arg) {
	if (soap_element_begin_in(soap, "LogoutResponse", 1, NULL) || soap_element_end_in(soap, "LogoutResponse"))
                return 1;

	return SOAP_OK;
}

SOAPFUNCO(Logout,put_logout,struct ManagedObjectReference *,get_logout);
