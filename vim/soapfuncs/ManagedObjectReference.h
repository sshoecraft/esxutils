
#ifndef __VIM_ManagedObjectReference_H
#define __VIM_ManagedObjectReference_H

#include "soapfuncs.h"

#ifndef __MOR_DEFINED
struct ManagedObjectReference {
	char *value;
	char *type;
};
#define __MOR_DEFINED 1
#endif

int send_MOR(struct soap *soap, char *tag, struct ManagedObjectReference *obj);
int put_ManagedObjectReference(struct soap *soap, char *tag, void **ptr, int req);
int recv_MOR(struct soap *soap, char *tag, struct ManagedObjectReference **mo_ref, int req);
int get_ManagedObjectReference(struct soap *soap, char *tag, void **ptr, int req);

#endif
