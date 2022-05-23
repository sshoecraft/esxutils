
#ifndef __VIM_CreateFolder_H
#define __VIM_CreateFolder_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct CreateFolderRequest {
	struct ManagedObjectReference *folder;
	char *name;
};

SOAPFUNCDEFIO(CreateFolder,struct CreateFolderRequest *,struct ManagedObjectReference **);

#endif
