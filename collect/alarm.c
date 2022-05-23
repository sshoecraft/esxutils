
#include "collect.h"
#include "gendb_host_alarms.h"

//#define LOG_ACTIONS 1
#ifdef LOG_ACTIONS
#include <stdarg.h>
#endif

struct alarm {
	char object[64];
	char name[256];
};

static list alarms = (list) 0;

static struct alarm *add_alarm(list alarms, struct alarm *new_alarm) {
	struct alarm *alarm;

	list_reset(alarms);
	while((alarm = list_get_next(alarms)) != 0) {
		if (strcmp(alarm->object,new_alarm->object) == 0) {
			return alarm;
		}
	}
	dprintf("adding alarm: %s\n", new_alarm->name);
	alarm = list_add(alarms, new_alarm, sizeof(*new_alarm));

	return alarm;
}

int get_alarm_info(struct csession *s, struct alarm *alarm, struct returnval *ret) {
	char *name, *p;

	memset(alarm,0,sizeof(*alarm));
	sprintf(alarm->object,"%s:%s", ret->obj.type, ret->obj.value);
	name = GETVAL("info.name");
	if (name) strcpy(alarm->name, name);

	return 0;
}

struct alarm *get_alarm(struct csession *s, char *object) {
	char *paths[] = {
		"info.name",
		0
	};
	struct alarm *alarm, new_alarm;
	list results;
	struct returnval *ret;
	struct ManagedObjectReference *mo_ref;

	dprintf("object: %s\n", object);

	list_reset(alarms);
	while((alarm = list_get_next(alarms)) != 0) {
		if (strcmp(alarm->object,object) == 0) {
			dprintf("found.\n");
			return alarm;
		}
	}
	dprintf("not found.\n");

	mo_ref = str2mor(object);
	results = vim_getinfo(s->vim, mo_ref->type, paths, 0, mo_ref);
	if (list_count(results) != 1) {
		printf("get_alarm(%s): result count != 1!\n", object);
		return 0;
	}
	free_mor(mo_ref);
	ret = results->first->item;
	if (get_alarm_info(s,&new_alarm, ret))
		return 0;
	return add_alarm(alarms, &new_alarm);
}

int get_alarms(struct csession *s, struct host *host) {
	char object[64];
	char *paths[] = {
		"summary.runtime.inMaintenanceMode",
		"declaredAlarmState",
		"configStatus",
		"configIssue",
		0
	};
	struct alarm *alarm;
	struct ManagedObjectReference *mo_ref;
	list results;
	struct returnval *ret;
	struct propSet *set,*set2,*set3;
	char *confstat, *p;

	if (!alarms) alarms = list_create();

	dprintf("get_alarms: host: %s\n", host->name);
	mo_ref = str2mor(host->object);
	results = vim_getinfo(s->vim, mo_ref->type, paths, 0, mo_ref);
	dprintf("count: %d\n", list_count(results));
	if (list_count(results) != 1) {
		printf("get_alarm(%s): result count != 1!\n", host->object);
		return 0;
	}
	free_mor(mo_ref);
	ret = results->first->item;

	/* Skip collecting alarms if host in maint mode */
	p = get_result_value(ret->nodes,"summary.runtime.inMaintenanceMode");
	if (p) {
		dprintf("inMaintenanceMode: %s\n", p);
		if (strcmp(p,"true") == 0) {
			dprintf("returning!\n");
			return 0;
		}
	}

	confstat = GETVAL("configStatus");
	if (confstat) {
		dprintf("configStatus: %s\n", confstat);
		if (strcmp(confstat,"green") != 0) {
			p = get_result_value(ret->nodes, "configIssue.Event.message");
			if (p) {
				dprintf("configIssue: %s\n", p);
				list_add(host->alarms,p,strlen(p)+1);
			} else {
				p = get_result_value(ret->nodes, "configIssue.Event.fullFormattedMessage");
				if (p) {
					dprintf("configIssue: %s\n", p);
					list_add(host->alarms,p,strlen(p)+1);
				} else {
					printf("unable to get configIssue for host %s\n", host->name);
				}
			}
		}
	}

	set = get_result(ret->nodes, "declaredAlarmState");
	if (!set) return 0;
	dprintf("set->name: %s\n", set->name);
	list_reset(set->nodes);
	while((set2 = list_get_next(set->nodes)) != 0) {
		dprintf("set2->name: %s\n", set2->name);
		p = get_result_value(set2->nodes, "overallStatus");
		dprintf("p: %s\n", p);
		if (strcmp(p,"green") == 0 || strcmp(p,"gray") == 0) continue;
		set3 = get_result(set2->nodes, "alarm");
		if (!set3) {
			dprintf("set3 is null!\n");
			continue;
		}
		sprintf(object,"%s:%s", set3->type, set3->value);
		alarm = get_alarm(s, object);
		if (!alarm) continue;
		dprintf("adding alarm: %s\n", alarm->name);
		list_add(host->alarms,alarm->name,strlen(alarm->name)+1);
	}

	return 0;
}

void get_host_alarms(struct csession *s, int host_id, list alarms) {
	struct host_alarms_record rec;
	char clause[1024];


	sprintf(clause," WHERE host_id = %d", host_id);
	if (host_alarms_select(s->db, clause)) return;
	while(host_alarms_fetch_record(s->db,&rec) == 0) {
		list_add(alarms, rec.alarm, strlen(rec.alarm)+1);
	}
}

#ifdef LOG_ACTIONS
void log_action(char *format, ...) {
	char ts[32], msg[1024], *p;
	va_list ap;
	FILE *fp;

	if (!actions_file) return;
	get_timestr(0,ts,sizeof(ts));
	va_start(ap,format);
	vsprintf(msg,format,ap);
	va_end(ap);
	for(p=msg; *p; p++) { if (isspace(*p)) *p = ' '; }
	trim(msg);
	fp = fopen(actions_file,"a+");
	if (fp) {
		fprintf(fp,"%s %s\n", ts, msg);
		fclose(fp);
	}
}
#endif

int sync_alarms(struct csession *s, struct host *host) {
	char temp[512], *p, *p2;
	list alarms;
	int found;

	/* Get a list of the alarms in the db */
	alarms = list_create();
	get_host_alarms(s, host->id, alarms);

	/* If no current alarms and no db alarms, return */
	dprintf("host->alarms: %d, db->alarms: %d\n", list_count(host->alarms),list_count(alarms));
	if (list_count(host->alarms) == 0 && list_count(alarms) == 0) return 0;

	/* Compare current with DB */
	list_reset(host->alarms);
	while((p = list_get_next(host->alarms)) != 0) {
		for(p2 = p; *p2; p2++) {
			if (*p2 == '\'')
				*p2 = ' ';
		}
		found = 0;
		while((p2 = list_get_next(alarms)) != 0) {
			temp[0] = 0;
			strncat(temp,p2,sizeof(temp)-1);
			dprintf("temp: %s, p2: %s\n", temp, p2);
			if (strcmp(temp,p2) == 0) {
				found = 1;
				break;
			}
		}
		if (!found) {
			struct host_alarms_record rec;

			memset(&rec,0,sizeof(rec));
			rec.host_id = host->id;
			strncat(rec.alarm,p,sizeof(rec.alarm)-1);
			strncat(rec.status,host->status,sizeof(rec.status)-1);
#ifdef LOG_ACTIONS
			log_action("adding alarm: host: %s, alarm: %s\n", host->name, rec.alarm);
#endif
			host_alarms_insert(s->db, &rec);
		}
	}

	/* Compare DB with current */
	list_reset(alarms);
	while((p = list_get_next(alarms)) != 0) {
		found = 0;
		list_reset(host->alarms);
		while((p2 = list_get_next(host->alarms)) != 0) {
			temp[0] = 0;
			strncat(temp,p2,sizeof(temp)-1);
			dprintf("p: %s, temp: %s\n", p, temp);
			if (strcmp(p,temp) == 0) {
				found = 1;
				break;
			}
		}
		if (!found) {
			char query[1024];

#ifdef LOG_ACTIONS
			log_action("deleting alarm: host: %s, alarm: %s\n", host->name, p);
#endif
			sprintf(query,"DELETE FROM host_alarms WHERE host_id = %d AND alarm = '%s'", host->id, p);
			db_exec(s->db, query);
		}
	}

	return 0;
}
