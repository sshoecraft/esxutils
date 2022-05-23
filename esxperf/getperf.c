
#include "vim.h"
#include "db.h"

static inline int _getval(struct vim_perfdata *data, int i) {
	int value;

	value = (data ? data[i].value : 0);
	if (value < 0) value = 0;
	return value;
}

int get_host_metrics(struct vim_session *s, DB *db, char *name, struct ManagedObjectReference *mo_ref, int *host_id) {
	char query[4096],*beginTime;
	struct vim_perfmetric host_metrics[] = {
		{ "usagemhz", 	"cpu", 	"megaHertz", 		"average" },
		{ "consumed", 	"mem", 	"kiloBytes",		"average" },
		{ "usage", 	"disk", "kiloBytesPerSecond",	"average" },
		{ "usage", 	"net",	"kiloBytesPerSecond",	"average" },
		{ 0,0,0,0 }
	};
	int i, count, rcount, cpu, mem, disk, net;

	sprintf(query,"SELECT id FROM hosts WHERE name = \'%s\'", name);
	dprintf("query: %s\n", query);
	if (db_exec(db,query) == 0 && db_fetch(db) == 0) {
		SQLGetData(db->hstmt, 1, SQL_INTEGER, host_id, sizeof(*host_id),0);
		db_fetch_done(db);
	}
	dprintf("host_id: %d\n", *host_id);
	if (!*host_id) {
		printf("error: unable to get host_id for %s\n", name);
		return -1;
	}

	beginTime = 0;
	sprintf(query,"SELECT MAX(time) FROM host_perf WHERE host_id = %d", *host_id);
	if (db_exec(db,query) == 0 && db_fetch(db) == 0) {
		SQLRETURN ret;
		query[0] = 0;

		ret = SQLGetData(db->hstmt, 1, SQL_C_CHAR, (SQLPOINTER) query, sizeof(query), 0);
		if (ret == 0) {
			query[10] = 'T';
			strcat(query,"Z");
			beginTime = query;
		}
		db_fetch_done(db);
	}
	if (beginTime) dprintf("beginTime: %s\n", beginTime);

	rcount = 0;
	if (vim_getperfmetrics(s, mo_ref, beginTime, host_metrics, 0) == 0) {
		count = host_metrics[0].count;
		dprintf("count: %d\n", count);
#if 0
		if (!count) {
			vim_getperfmetrics(s, mo_ref, beginTime, host_metrics, 1);
			count = host_metrics[0].count;
			dprintf("count: %d\n", count);
		}
#endif
		_db_disperr = 0;
		for(i=0; i < count; i++) {
			if (!host_metrics[0].data) continue;
			cpu = _getval(host_metrics[0].data,i);
			mem = _getval(host_metrics[1].data,i);
			disk = _getval(host_metrics[2].data,i);
			net = _getval(host_metrics[3].data,i);
			if (cpu == 0 && mem == 0 && disk == 0 && net == 0) continue;
			sprintf(query,"INSERT INTO host_perf (time,host_id,cpu,mem,disk,net) VALUES (\"%s\",%d,%d,%d,%d,%d)",
				host_metrics[0].data[i].timeStamp,		/* time (from cpu) */
				*host_id,					/* host_id */
				cpu,mem,disk,net
			);
//			if (db_exec(db,query)) printf("bad insert: %s\n",query);
			db_exec(db,query);
			rcount++;
		}
		_db_disperr = 1;
	} else {
		printf("unable to get perf metrics for: %s\n", name);
	}
	return rcount;
}

int get_vm_metrics(struct vim_session *s, DB *db, char *name, struct ManagedObjectReference *mo_ref, int host_id) {
	char query[4096], *beginTime;
	struct vim_perfmetric vm_metrics[] = {
		{ "usagemhz", 	"cpu", 	"megaHertz", 		"average" },
		{ "ready",	"cpu",	"millisecond",		"summation" },
		{ "consumed", 	"mem", 	"kiloBytes",		"average" },
		{ "usage", 	"disk", "kiloBytesPerSecond",	"average" },
		{ "usage", 	"net",	"kiloBytesPerSecond",	"average" },
		{ 0,0,0,0 }
	};
	int vm_id, i, count, rcount, cpu, ready, mem, disk, net;

	sprintf(query,"SELECT id FROM vms WHERE name = \'%s\' and host_id = %d", name, host_id);
	dprintf("query: %s\n", query);
	if (db_exec(db,query) == 0 && db_fetch(db) == 0) {
		SQLGetData(db->hstmt, 1, SQL_INTEGER, &vm_id, sizeof(vm_id), 0);
		db_fetch_done(db);
	}
	dprintf("vm_id: %d\n", vm_id);
	if (!vm_id) {
		printf("error: unable to get vm_id for %s\n", name);
		return -1;
	}

	beginTime = 0;
	sprintf(query,"SELECT MAX(time) FROM vm_perf WHERE vm_id = %d",vm_id);
	if (db_exec(db,query) == 0 && db_fetch(db) == 0) {
		SQLRETURN ret;
		query[0] = 0;

		ret = SQLGetData(db->hstmt, 1, SQL_C_CHAR, (SQLPOINTER) query, sizeof(query), 0);
		dprintf("ret: %d\n", (int)ret);
		if (ret == 0) {
			query[10] = 'T';
			strcat(query,"Z");
			beginTime = query;
		}
		db_fetch_done(db);
	}
	if (beginTime) dprintf("beginTime: %s\n", beginTime);

	rcount = 0;
	if (vim_getperfmetrics(s, mo_ref, beginTime, vm_metrics, 0) == 0) {
		count = vm_metrics[0].count;
		dprintf("count: %d\n", count);
#if 0
		if (!count) {
			vim_getperfmetrics(s, mo_ref, beginTime, vm_metrics, 1);
			count = vm_metrics[0].count;
			dprintf("count: %d\n", count);
		}
#endif
		_db_disperr = 0;
		for(i=0; i < count; i++) {
			if (!vm_metrics[0].data) continue;
			cpu = _getval(vm_metrics[0].data, i);
			ready = _getval(vm_metrics[1].data, i);
			mem = _getval(vm_metrics[2].data, i);
			disk = _getval(vm_metrics[3].data, i);
			net = _getval(vm_metrics[4].data, i);
			if (cpu == 0 && mem == 0 && disk == 0 && net == 0) continue;
			/* XXX fixup ready */
			if (ready) {
				char temp[16];
				float val;

//				printf("before: %d\n", ready);
				val = ((float)ready/15.0);
				sprintf(temp,"%.0f", val);
				ready = atoi(temp);
//				printf("after: %d\n", ready);
			}
			sprintf(query,"INSERT INTO vm_perf (time,vm_id,cpu,ready,mem,disk,net) VALUES (\"%s\",%d,%d,%d,%d,%d,%d)",
				vm_metrics[0].data[i].timeStamp,		/* time (from cpu) */
				vm_id,						/* vm_id */
				cpu,ready,mem,disk,net
			);
//			if (db_exec(db,query)) printf("bad insert: %s\n", query);
			db_exec(db,query);
			rcount++;
		}
		_db_disperr = 1;
	} else {
		printf("unable to get perf metrics for: %s\n", name);
	}
	return rcount;
}
