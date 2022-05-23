
#include "esxpost.h"
#include "soapfuncs/ServiceContent.h"
#include "soapfuncs/GetAlarm.h"
#include "soapfuncs/SetAlarmStatus.h"
#include "soapfuncs/RetrieveProperties.h"
#include "list.h"

/* XXX 600 was not enough ... ? */
/* XXX auto disconnected @ 640 */
#define TIMEOUT 90

int clear(struct esxpost *info) {
	struct SetAlarmStatusRequest sareq;
	struct ManagedObjectReference *host_obj,alarm_obj;
	char *paths[] = { "declaredAlarmState", 0 }, *p;
	list results;
	struct returnval *ret;
	struct propSet *set,*set2,*set3;
	int r;

	dprintf("calling findobj... vim: %p\n", info->vim);
	host_obj = vim_findobj(info->vim, "HostSystem", info->fqdn);
	dprintf("host_obj: %p\n", host_obj);
	if (!host_obj) {
		printf("error: clear: host object not found? ...\n");
		return 1;
	}

	if (vim_isconnected(info->vim,host_obj) == 0 && no == 0) {
		int timeout;

		printf("Waiting for host to become connected...\n");
		for(timeout=TIMEOUT; timeout > 0; timeout -= 10) {
			dprintf("timeout: %d\n", timeout);
			if (vim_isconnected(info->vim,host_obj)) break;
			sleep(10);
		}
		if (!timeout) {
			printf("timeout waiting for host to become connected, reconnecting!\n");
			if (con_host(info,host_obj)) {
				printf("error clear: reconnecting host!\n");
//				return 1;
			}
		}
	}

	printf("Clearing alarms....\n");
	results = vim_getinfo(info->vim, host_obj->type, paths, 0, host_obj);
	dprintf("count: %d\n", list_count(results));
	if (list_count(results) != 1) {
		printf("get_alarm(%s): result count != 1!\n", host_obj->value);
		return 0;
	}
	ret = results->first->item;

	/* Skip collecting alarms if host in maint mode */
	set = get_result(ret->nodes, "declaredAlarmState");
	if (!set) return 0;
	dprintf("set->name: %s\n", set->name);
	list_reset(set->nodes);
	while((set2 = list_get_next(set->nodes)) != 0) {
//		dprintf("set2->name: %s\n", set2->name);
		p = get_result_value(set2->nodes, "overallStatus");
//		dprintf("p: %s\n", p);
		if (!p) continue;
		if (strcmp(p,"green") == 0 || strcmp(p,"gray") == 0) continue;
		set3 = get_result(set2->nodes, "alarm");
		if (!set3) continue;
		alarm_obj.type = set3->type;
		alarm_obj.value = set3->value;

		printf("Clearing alarm %s:%s from %s...\n", alarm_obj.type, alarm_obj.value, info->fqdn);
		dprintf("info->vim->sc->alarmManager: %p\n",info->vim->sc->alarmManager);
		if (!info->vim->sc->alarmManager) {
			printf("error: clear: alarmManager is null!\n");
			return 1;
		}
		sareq.alarmManager = info->vim->sc->alarmManager;
		sareq.alarm = &alarm_obj;
		sareq.entity = host_obj;
		sareq.status = "green";
		dprintf("Calling SetAlarmStatus...\n");
		r = SetAlarmStatus(info->vim->soap,info->vim->endpoint,&sareq);
		dprintf("r: %d\n", r);
	}

	return 0;
}
