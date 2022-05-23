
#include <unistd.h>
#include "vim.h"
#include "RetrieveProperties.h"
#include "EnterMaintenanceMode.h"
#include "ExitMaintenanceMode.h"
#include "TaskInfo.h"

int vim_ismm(struct vim_session *vim, struct ManagedObjectReference *mo_ref) {
	list results;
	struct returnval *ret;
	struct propSet *set;
	char *paths[] = {
		"runtime.inMaintenanceMode",
		0
	};
	int found,r;

	r = 0;
        results = vim_getinfo(vim, mo_ref->type, paths, 0, mo_ref);
	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
		dprintf("ret->obj: %s:%s\n", ret->obj.type, ret->obj.value);
		list_reset(ret->nodes);
		while((set = list_get_next(ret->nodes)) != 0) {
			dprintf("set->name: %s\n", set->name);
			dprintf("set->value: %s\n", set->value);
			if (strcmp(set->name,"runtime.inMaintenanceMode") == 0) {
				if (strcmp(set->value,"true") == 0) r = 1;
				found = 1;
				break;
			}
		}
		if (found) break;
	}

	dprintf("r: %d\n", r);
	return r;
}

int vim_entermm(struct vim_session *vim, struct ManagedObjectReference *mo_ref, int timeout) {
	struct EnterMaintenanceModeRequest req;
	struct ManagedObjectReference *task;
	struct TaskInfo *info;
	list tasks;

	/* Check if already in maint mode */
	if (vim_ismm(vim,mo_ref)) return 0;

	/* Get all running tasks */
	tasks = vim_gettasksbystate(vim, mo_ref, "running");
	dprintf("tasks: %p\n", tasks);
	if (tasks) {
		/* If one running, use that ... */
		info = vim_findtask(tasks,"EnterMaintenanceMode_Task");
		if (info) return vim_wait4task(vim,info->task,timeout);
	}

	/* Call enter */
//	dprintf("obj: %s:%s\n", mo_ref->type, mo_ref->value);
	req.obj = mo_ref;
	req.timeout = 0;
	req.evacuatePoweredOffVms = 1;
	if (EnterMaintenanceMode(vim->soap, vim->endpoint, &req, &task)) return 1;

	/* Wait for task */
	return vim_wait4task(vim,task,timeout);
}

int vim_exitmm(struct vim_session *vim, struct ManagedObjectReference *mo_ref, int timeout) {
	struct ExitMaintenanceModeRequest req;
	struct ManagedObjectReference *task;
	struct TaskInfo *info;
	list tasks;

	/* Check if not in maint mode */
	if (!vim_ismm(vim,mo_ref)) return 0;

	/* Get all running tasks */
	tasks = vim_gettasksbystate(vim, mo_ref, "running");
	dprintf("tasks: %p\n", tasks);
	if (tasks) {
		/* If one running, use that ... */
		info = vim_findtask(tasks,"ExitMaintenanceMode_Task");
		if (info) return vim_wait4task(vim,info->task,timeout);
	}

	/* Call enter */
//	dprintf("obj: %s:%s\n", mo_ref->type, mo_ref->value);
	req.obj = mo_ref;
	req.timeout = 0;
	if (ExitMaintenanceMode(vim->soap, vim->endpoint, &req, &task)) return 1;

	/* Wait for task */
	return vim_wait4task(vim,task,timeout);
}
