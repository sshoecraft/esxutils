
#include "DynamicProperty.h"

int get_DynamicProperty(struct soap *soap, char *tag, void **ptr, int req) {
	struct DynamicProperty *prop = *ptr;
	if (prop) {
		struct mydesc get_dp_desc[] = {
			{ "name", &prop->name, get_char, 0 },
			{ "val", &prop->val, get_char, 0 },
			{ 0,0,0,0 }
		};
		return soap_recv_desc(soap,tag,get_dp_desc);
	}
	return 1;
}

int get_DynamicData(struct soap *soap, char *tag, void **ptr, int req) {
	struct DynamicData *data = *ptr;
#if 0
	struct mydesc get_dd_desc[] = {
		{ 0,0,0,0 }
	};
#endif
	memset(data,0,sizeof(data));
	return 0;
}
