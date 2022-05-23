
#ifndef __VIM_TaskFilterSpec_H
#define __VIM_TaskFilterSpec_H

#include "soapfuncs.h"
#include "list.h"
#include "ManagedObjectReference.h"

enum TaskFilterSpecRecursionOption {
	TaskFilterSpecRecursionOption_self = 0,
	TaskFilterSpecRecursionOption_children = 1,
	TaskFilterSpecRecursionOption_all = 2
};

enum TaskFilterSpecTimeOption {
	TaskFilterSpecTimeOption_queuedTime = 0,
	TaskFilterSpecTimeOption_startedTime = 1,
	TaskFilterSpecTimeOption_completedTime = 2
};

enum TaskInfoState {
	TaskInfoState_queued = 0,
	TaskInfoState_running = 1,
	TaskInfoState_success = 2,
	TaskInfoState_error = 3
};

//	enum TaskFilterSpecRecursionOption recursion;      /* required element of type ns2:TaskFilterSpecRecursionOption */
struct TaskFilterSpecByEntity {
	struct ManagedObjectReference *entity;
//	int recursion;
	char *recursion;
};

struct TaskFilterSpec {
//	struct ManagedObjectReference *alarm;		/* Optional */
	struct TaskFilterSpecByEntity *entity;		/* Optional */
//	struct ManagedObjectReference *scheduledTask;	/* Optional */
	list state;					/* Optional */
//	struct TaskFilterSpecByTime *time;		/* Optional */
//	struct TaskFilterSpecByUsername *user;		/* Optional */
};

int put_TaskFilterSpec(struct soap *soap, char *tag, void **ptr, int req);

#endif
