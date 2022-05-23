
#if 0
#include "LocalizedMethodFault.h"

int get_LocalizedMethodFault(struct soap *soap, char *tag, void **ptr, int req) {
	struct LocalizedMethodFault *fault = *ptr;
	if (fault) {
		struct mydesc get_lmf_desc[] = {
			{ "fault", &fault->fault, get_DynamicData, 0 },
			{ "localizedMessage", &fault->localizedMessage, get_char, 0 },
			{ 0,0,0,0 }
		};
		return soap_recv_desc(soap,tag,get_lmf_desc);
	}
	return 0;
}
#endif
