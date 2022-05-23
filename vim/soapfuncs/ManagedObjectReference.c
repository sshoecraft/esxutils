

#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG 1

#include "ManagedObjectReference.h"

int send_MOR(struct soap *soap, char *tag, struct ManagedObjectReference *obj) {
	if (!obj) return 1;
	return (soap_element(soap,tag,0,0) ||
			soap_attribute(soap, "type", obj->type) ||
			soap_element_start_end_out(soap, 0) ||
			soap_string_out(soap, obj->value, 0) || 
			soap_element_end_out(soap, tag));
}

int put_ManagedObjectReference(struct soap *soap, char *tag, void **ptr, int req) {
	struct ManagedObjectReference *obj = *ptr;
	dprintf("obj: %p, req: %d\n", obj, req);
	if (!obj && !req) return 0;
	return send_MOR(soap,tag,obj);
}

int recv_MOR(struct soap *soap, char *tag, struct ManagedObjectReference **mo_ref, int req) {
//	void **ptr = (void **)mo_ref;
	int r;

	dprintf("tag: %s\n", tag);
	*mo_ref = soap_malloc(soap, sizeof(struct ManagedObjectReference));
	if (!*mo_ref) {
		dprintf("get_ManagedObjectReference: malloc error\n");
		return 1;
	}
	r = 0;
//	*mor = *ptr;
	(*mo_ref)->type = (*mo_ref)->value = 0;
	if (soap_element_begin_in(soap, tag, 1, NULL)
			|| soap_s2string(soap, soap_attr_value(soap, "type", 0), &(*mo_ref)->type, -1, -1)
			|| !soap_instring(soap, "-", &(*mo_ref)->value, "xsd:string", 0, 1, -1, -1)
			|| soap_element_end_in(soap, tag)) {
		r = 1;
	}
	dprintf("type: %s, value: %s\n", (*mo_ref)->type, (*mo_ref)->value);
	dprintf("req: %d, r: %d\n", req, r);
	return (req && r ? 1 : 0);
}

int get_ManagedObjectReference(struct soap *soap, char *tag, void **ptr, int req) {
	return recv_MOR(soap,tag,(struct ManagedObjectReference **)ptr,req);
}
