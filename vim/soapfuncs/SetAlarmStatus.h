
#ifndef __VIM_SetAlarmStatus_H
#define __VIM_SetAlarmStatus_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct SetAlarmStatusRequest {
	struct ManagedObjectReference *alarmManager;
	struct ManagedObjectReference *alarm;
	struct ManagedObjectReference *entity;
	char *status;
};

SOAPFUNCDEFO(SetAlarmStatus,struct SetAlarmStatusRequest *);

#endif
