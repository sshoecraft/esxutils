
#include "ReadNextTasks.h"

int put_rnt(struct soap *soap, void *arg) {
	struct ReadNextTasksRequest *req = arg;
	struct mydesc put_rnt_desc[] = {
		{ "_this", &req->obj, put_ManagedObjectReference, 0 },
		{ "maxCount", &req->maxCount, put_int, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"ReadNextTasks","xmlns",MYNS,put_rnt_desc);
}

int get_taskInfo(struct soap *soap, char *tag, list results) {
	struct TaskInfo task;
	struct mydesc get_task_desc[] = {
		{ "key", &task.key, get_char, 0 },
		{ "task", &task.task, get_char, 0 },
		{ "name", &task.name, get_char, 0 },
		{ "descriptionId", &task.descriptionId, get_char, 0 },
		{ "entity", &task.entity, get_ManagedObjectReference, 0 },
		{ "entityName", &task.entityName, get_char, 0 },
		{ "state", &task.state, get_char, 0 },
		{ "progress", &task.progress, get_int, 0 },
		{ "cancelled", &task.cancelled, get_bool, 0 },
		{ "cancelable", &task.cancelable, get_bool, 0 },
		{ "result", &task.state, get_char, 0 },
//		{ "reason", &task.reason, get_DynamicProperty, 0 },
		{ "queueTime", &task.queueTime, get_char, 0 },
		{ "startTime", &task.startTime, get_char, 0 },
		{ "completeTime", &task.completeTime, get_char, 0 },
		{ "eventChainId", &task.eventChainId, get_int, 0 },
		{ 0,0,0,0 }
	};

	return soap_recv_desc(soap,tag,get_task_desc);
}


int get_rnt(struct soap *soap, void *arg) {
	list results = arg;
	char *tag = "ReadNextTasksResponse";
	int count,r;

	/* must have <tag> */
	if (soap_element_begin_in(soap, tag, 0, 0)) {
		dprintf("soap_element_begin_in(%s) failed\n",tag);
		return 1;
	}
	dprintf("got <%s>\n",tag);

	count = 0;
	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"returnval") == 0) {
#if 0
	while(1) {
		if (soap_peek_element(soap) != SOAP_OK) {
			dprintf("peek error\n");
			break;
		}
#endif
		dprintf("peeked element: %s\n", soap->tag);
		if (get_taskInfo(soap, "returnval", results))
			goto get_rnt_error;
		count++;
	}

	/* must have </tag> */
	if (soap_element_end_in(soap, tag)) {
		dprintf("soap_element_end_in(%s) failed\n",tag);
		return 1;
	}
	dprintf("got </%s>\n",tag);

	r = (count > 0 ? 0 : 1);
	dprintf("returning: %d\n",r);
	return r;

get_rnt_error:
	soap_element_end_in(soap, tag);
	return 1;
}

SOAPFUNCIO(ReadNextTasks,put_rnt,struct ReadNextTasksRequest *,get_rnt,list);
