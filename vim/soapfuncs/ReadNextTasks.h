
#ifndef __VIM_ReadNextTasks_H
#define __VIM_ReadNextTasks_H

#include "list.h"
#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "TaskInfo.h"

struct ReadNextTasksRequest {
	struct ManagedObjectReference *obj;
	int maxCount;
};

SOAPFUNCDEFIO(ReadNextTasks,struct ReadNextTasksRequest *,list);

#endif
