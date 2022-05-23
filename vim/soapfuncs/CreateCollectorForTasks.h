
#ifndef __VIM_CreateCollectorForTasks_H
#define __VIM_CreateCollectorForTasks_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "TaskFilterSpec.h"

struct CreateCollectorForTasksRequest {
	struct ManagedObjectReference *taskManager;
	struct TaskFilterSpec *filter;
};

SOAPFUNCDEFIO(CreateCollectorForTasks,struct CreateCollectorForTasksRequest *,struct ManagedObjectReference **);

#endif
