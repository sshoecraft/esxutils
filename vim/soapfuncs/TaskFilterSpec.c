
#include "TaskFilterSpec.h"

static int put_TaskFilterSpecByEntity(struct soap *soap, char *tag, void **ptr, int req) {
	struct TaskFilterSpecByEntity *entity = *ptr;
	struct mydesc put_entity_desc[] = {
		{ "entity", &entity->entity, put_ManagedObjectReference, 0 },
		{ "recursion", &entity->recursion, put_char, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,tag,"xsi:type","TaskFilterSpecByEntity",put_entity_desc);
}

static int put_TaskInfoStateList(struct soap *soap, char *tag, void **ptr, int req) {
	list states = *ptr;
	char *state;

	list_reset(states);
	while((state = list_get_next(states)) != 0) {
		soap_send_string(soap,"state",state);
	}
	return 0;
}

int put_TaskFilterSpec(struct soap *soap, char *tag, void **ptr, int req) {
	struct TaskFilterSpec *filter = *ptr;
	struct mydesc put_filter_desc[] = {
//		{ "alarm", &filter->alarm, put_ManagedObjectReference, 0 },
		{ "entity", &filter->entity, put_TaskFilterSpecByEntity, 0 },
//		{ "scheduledTask", &filter->scheduledTask, put_ManagedObjectReference, 0 },
//		{ "TaskInfoState", &filter->TaskInfoState, put_TaskInfoState, 0 },
//		{ "time", &filter->time, put_TaskFilterSpecByTime, 0 },
//		{ "user", &filter->user, put_TaskFilterSpecByUsername, 0 },
		{ "state", &filter->state, put_TaskInfoStateList, 0 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,tag,"xsi:type","TaskFilterSpec",put_filter_desc);
}
