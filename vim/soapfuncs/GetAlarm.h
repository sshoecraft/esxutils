
#ifndef __VIM_GetAlarm_H
#define __VIM_GetAlarm_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "list.h"

struct GetAlarmRequest {
	struct ManagedObjectReference *alarmManager;
	struct ManagedObjectReference *entity;
};

SOAPFUNCDEFIO(GetAlarm,struct GetAlarmRequest *,list);

#endif
