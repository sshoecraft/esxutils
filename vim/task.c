
#include "ivim.h"
#include "ServiceContent.h"
#include "RetrieveProperties.h"
#include "TaskInfo.h"
#include "CreateCollectorForTasks.h"
#include "ReadNextTasks.h"
#include "DestroyCollector.h"

#if 0
CancelTask - Cancels a running or queued task
SetTaskState - Sets task state and optionally sets results or fault, as appropriate for state 
UpdateProgress - Sets percentage done for this task and recalculates overall percentage done. 
ReadNextTasks, ReadPreviousTasks 
#endif

#include <unistd.h>
#include "vim.h"

static void _gettaskinfo(struct vim_session *vim, list nodes, struct TaskInfo *info) {
	struct vim_res2desc TaskInfo_desc[] = {
		{ "key", VIM_TYPE_PSTRING, &info->key, 0, 0, 1 },
		{ "task", VIM_TYPE_POBJ, &info->task, 0, 0, 1 },
		{ "name", VIM_TYPE_PSTRING, &info->name, 0, 0, 0 },
		{ "description", VIM_TYPE_PSTRING, &info->description, 0, 0, 0 },
		{ "descriptionId", VIM_TYPE_PSTRING, &info->descriptionId, 0, 0, 0 },
		{ "entity", VIM_TYPE_OBJ, &info->entity, 0, 0, 0 },
		{ "entityName", VIM_TYPE_PSTRING, &info->entityName, 0, 0, 0 },
		{ "state", VIM_TYPE_PSTRING, &info->state, 0, 0, 0 },
		{ "progress", VIM_TYPE_INT, &info->progress, 0, 0, 0 },
		{ "cancelled", VIM_TYPE_BOOL, &info->cancelled, 0, 0, 0 },
		{ "cancelable", VIM_TYPE_BOOL, &info->cancelable, 0, 0, 0 },
		{ "fault", VIM_TYPE_PSTRING, &info->fault, 0, 0, 0 },	
		{ "faultMessage", VIM_TYPE_PSTRING, &info->faultMessage, 0, 0, 0 },
		{ "userName", VIM_TYPE_PSTRING, &info->userName, 0, 0, 0 },	
		{ "queueTime", VIM_TYPE_PSTRING, &info->queueTime, 0, 0, 0 },	
		{ "startTime", VIM_TYPE_PSTRING, &info->startTime, 0, 0, 0 },	
		{ "completeTime", VIM_TYPE_PSTRING, &info->completeTime, 0, 0, 0 },	
		{ "eventChainId", VIM_TYPE_INT, &info->eventChainId, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};

	vim_results2desc(vim, nodes, TaskInfo_desc);
}

static void _results2tasks(struct vim_session *vim, list nodes, list tasks) {
	struct TaskInfo info;
	struct propSet *set;

	list_reset(nodes);
	while((set = list_get_next(nodes)) != 0) {
		dprintf("set->name: %s\n", set->name);
		dprintf("set->value: %s\n", set->value);
		dprintf("set->nodes: %d\n", list_count(set->nodes));
		if (strcmp(set->name,"TaskInfo") == 0) {
			memset(&info,0,sizeof(info));
			_gettaskinfo(vim, set->nodes, &info);
//			printf("info.name: %s, info.state: %s\n", info.name, info.state);
//			if (!info.name) exit(1);
			list_add(tasks,&info,sizeof(info));
		}
	}
}

list vim_gettasks(struct vim_session *vim, struct ManagedObjectReference *mo_ref, list states) {
	struct CreateCollectorForTasksRequest req;
	struct TaskFilterSpecByEntity entity;
	struct TaskFilterSpec filter;
	struct ManagedObjectReference *cobj;
	list results;
	struct returnval *ret;
	struct propSet *set;
	char *paths[] = {
		0
	};
	struct ReadNextTasksRequest rnreq;
	int count,r;
	list tasks;

	tasks = list_create();

	if (mo_ref) {
		memset(&entity,0,sizeof(entity));
		entity.entity = mo_ref;
		entity.recursion = "self";
	}

	memset(&filter,0,sizeof(filter));
	if (mo_ref) filter.entity = &entity;
	filter.state = states;

	req.taskManager = vim->sc->taskManager;
	req.filter = &filter;

	dprintf("Creating collector...\n");
	if (CreateCollectorForTasks(vim->soap,vim->endpoint,&req,&cobj)) return 0;

	results = vim_getinfo(vim,cobj->type,paths,1,cobj);
	count = list_count(results);
	dprintf("count: %d\n", count);
	if (count != 1) {
		printf("error: count from getinfo != 1\n");
		goto done;
	}

	ret = results->first->item;
	set = get_result(ret->nodes,"latestPage");
	if (!set) goto done;
	_results2tasks(vim,set->nodes,tasks);

	rnreq.obj = cobj;
	rnreq.maxCount = 1000;
	while(1) {
		r = ReadNextTasks(vim->soap,vim->endpoint,&rnreq,tasks);
		dprintf("r: %d\n", r);
		if (r) break;
	}

done:
	dprintf("Destroying collector...\n");
	DestroyCollector(vim->soap,vim->endpoint,cobj);
	dprintf("count: %d\n", list_count(tasks));
	return tasks;
}

list vim_gettasksbystate(struct vim_session *vim, struct ManagedObjectReference *mo_ref, char *state) {
	list states;

	states = list_create();
	if (state) list_add(states,state,strlen(state)+1);
	return vim_gettasks(vim,mo_ref,states);
}

struct TaskInfo *vim_findtask(list tasks, char *name) {
	struct TaskInfo *task;

	dprintf("name: %s\n", name);
	list_reset(tasks);
	while((task = list_get_next(tasks)) != 0) {
		dprintf("task: obj: %s:%s, name: %s, state: %s\n", task->task->type, task->task->value, task->name, task->state);
		if (!task->name) continue;
		if (strcmp(task->name,name) == 0) {
			dprintf("found.\n");
			return task;
		}
	}
	dprintf("not found\n");
	return 0;
}

int vim_wait4task(struct vim_session *vim, struct ManagedObjectReference *mo_ref, int timeout) {
	list results;
	char *paths[] = { 0 };
	int x,count,done,r;
	struct returnval *ret;
	struct propSet *set;
	struct TaskInfo info;

	dprintf("timeout: %d\n", timeout);

	done = r = 0;
	x = (timeout ? timeout : 31536000);
	while(x--) {
		dprintf("x: %d\n", x);
		results = vim_getinfo(vim,mo_ref->type,paths,1,mo_ref);
		if (!results) break;
		count = list_count(results);
		dprintf("count: %d\n", count);
		if (count < 1) break;
		ret = results->first->item;
		count = list_count(ret->nodes);
		dprintf("count: %d\n", count);
		if (count < 1) break;
		list_reset(ret->nodes);
		while((set = list_get_next(ret->nodes)) != 0) {
			dprintf("set: name: %s, type: %s, value: %s, nodes: %d\n", set->name, set->type, set->value, list_count(set->nodes));
			if (strcmp(set->name,"info") == 0) {
				memset(&info,0,sizeof(info));
				_gettaskinfo(vim, set->nodes, &info);
				dprintf("task: name: %s, state: %s\n", info.name, info.state);
				if (strcmp(info.state,"error") == 0) {
					done = r = 1;
					break;
				} if (strcmp(info.state,"running") != 0) {
					done = 1;
				}
				break;
			}
		}
		list_destroy(results);
		if (done) break;
#ifdef __MINGW32__
		Sleep(1000);
#else
		sleep(1);
#endif
	}
	dprintf("x: %d\n", x);
	if (x < 0) r = -2;
	dprintf("r: %d\n", r);
	return r;
}
