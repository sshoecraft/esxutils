
#ifndef __VIM_Rename_H
#define __VIM_Rename_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct RenameRequest {
	struct ManagedObjectReference *obj;
	char *newName;
};

SOAPFUNCDEFIO(Rename,struct RenameRequest *,struct ManagedObjectReference **);

#endif
