
#include "soapfuncs.h"
#include "ServiceContent.h"

struct ServiceContent *new_ServiceContent(struct soap *soap) {
	struct ServiceContent *sc;

	sc = soap_malloc(soap, sizeof(struct ServiceContent));
	if (sc) {
		sc->about = new_AboutInfo(soap);
		if (!sc->about) {
			soap_dealloc(soap,sc);
			sc = 0;
		}
	}
	dprintf("sc: %p\n", sc);
	return sc;
}
