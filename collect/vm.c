#include "collect.h"
#include "gendb_hosts.h"
#include "gendb_vms.h"
#include "gendb_vm_files.h"
#include "gendb_vm_disks.h"
#include "gendb_vm_hist.h"
#include "soapfuncs/RetrieveProperties.h"

static char **vm_paths;

static struct vm *add_vm(list vms, struct vm *new_vm) {
	struct vm *vm;

	list_reset(vms);
	while((vm = list_get_next(vms)) != 0) {
		if (strcmp(vm->object,new_vm->object) == 0) {
			return vm;
		}
	}
	dprintf("adding vm: %s\n", new_vm->name);
	vm = list_add(vms, new_vm, sizeof(*new_vm));
	dprintf("added.\n");

	return vm;
}

static void fixname(char *name) {
	register char *p;

	for(p=name; *p; p++) {
#if 0
// Dont do this - it gets rid of (2), etc
		if (isspace(*p)) {
			*p = 0;
			break;
		}
#endif
		if (*p == '\'')
			strcpy(p,p+1);
	}
}

static void get_ci_name(struct csession *s, struct vm *vm) {
	char query[128], temp[64], *p;
	int found;

	dprintf("name: %s\n", vm->name);
	strcpy(temp, vm->name);
	p = strchr(temp,'_');
	if (p) *p = 0;
	p = strchr(temp,' ');
	if (p) *p = 0;
	p = strchr(temp,'(');
	if (p) *p = 0;
	dprintf("temp: %s\n", temp);

	_db_disperr = 0;
	found = 0;
//	sprintf(query,"SELECT name FROM systems WHERE name like '%s%%' AND system_type_id in (select id from system_types where type = 'Virtual')", temp);
	sprintf(query,"SELECT name FROM systems WHERE name like '%s%%'", temp);
	if (db_exec(s->cidb,query) == 0) {
		if (db_fetch(s->cidb) == 0) {
			SQLGetData(s->cidb->hstmt, 1, SQL_C_CHAR, (SQLPOINTER) vm->ci_name, sizeof(vm->ci_name), 0);
			dprintf("ci_name: %s\n", vm->ci_name);
			found = 1;
		}
		db_fetch_done(s->cidb);
	}

	/* If not found, try DNS */
//	dprintf("found: %d\n", found);
	if (!found) {
		struct hostent *ent;

		dprintf("temp: %s\n", temp);
		ent = getdnshostent(temp);
		dprintf("ent: %p\n", ent);
		if (ent) {
			char *ci_name;

			/* If no domain in the host, and we have aliases ... */
			ci_name = "";
			if (!strchr(ent->h_name,'.') && ent->h_aliases) {
				int i;
				for(i=0; ent->h_aliases[i]; i++) {
					if (strchr(ent->h_aliases[i],'.')) {
						ci_name = ent->h_aliases[i];
						break;
					}
				}
			} else {
				ci_name = ent->h_name;
			}
			if (!ci_name) ci_name = "";
			dprintf("ci_name: %s\n", ci_name);
			strncpy(vm->ci_name,ci_name,sizeof(vm->ci_name)-1);
		}
	}
	_db_disperr = 1;
}

static int get_vm_info(struct csession *s, struct vm *vm, struct returnval *ret) {
	struct ManagedObjectReference *mo_ref = &ret->obj;
	char temp[64], *p;
//	char bt[32];
	struct propSet *set,*set2;
	struct vm_files_record new_file;
	struct vm_disks_record new_disk;
	struct ManagedObjectReference host_obj;
	struct vim_res2desc vm_desc[] = {
		{ "parent", VIM_TYPE_UNK, 0, 0, 0, 1 },
		{ "summary.config.name", VIM_TYPE_STRING, vm->name, sizeof(vm->name)-1, 0, 1 },
		{ "summary.config.uuid", VIM_TYPE_STRING, vm->uuid, sizeof(vm->uuid)-1, 0, 0 },
		{ "summary.config.numCpu", VIM_TYPE_INT, &vm->cpu_total, 0, 0, 0 },
		{ "summary.config.memorySizeMB", VIM_TYPE_INT, &vm->mem_total, 0, 0, 0 },
		{ "summary.config.guestId", VIM_TYPE_STRING, vm->os, sizeof(vm->os)-1, 0, 0 },
		{ "summary.config.annotation", VIM_TYPE_STRING, vm->annotation, sizeof(vm->annotation)-1, 0, 0 },
		{ "summary.runtime.host", VIM_TYPE_OBJ, &host_obj, sizeof(host_obj)-1, 0, 0 },
//		{ "summary.guest.guestId", VIM_TYPE_STRING, vm->os, sizeof(vm->os)-1, 0, 0 },
		{ "summary.guest.toolsStatus", VIM_TYPE_STRING, vm->tools, sizeof(vm->tools)-1, 0, 0 },
		{ "summary.runtime.powerState", VIM_TYPE_STRING, vm->state, sizeof(vm->state)-1, 0, 0 },
#ifdef BOOT_TIME
		{ "summary.runtime.bootTime", VIM_TYPE_STRING, vm->boot_time, sizeof(vm->boot_time)-1, 0, 0 },
#endif
		{ "summary.quickStats.overallCpuUsage",VIM_TYPE_INT, &vm->cpu_usage, 0, 0, 0 },
		{ "summary.quickStats.hostMemoryUsage",VIM_TYPE_INT, &vm->mem_usage, 0, 0, 0 },
//		{ "runtime.bootTime", VIM_TYPE_STRING, bt, sizeof(bt)-1, 0, 0 },
		{ "config.hardware.device", 0, 0, 0, 0, 0 },
		{ "guest.ipAddress", VIM_TYPE_STRING, vm->ip, sizeof(vm->ip)-1, 0, 0 },
		{ "layoutEx.file", 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};

	dprintf("vm: %p, ret: %p\n", vm, ret);
	if (vm == 0 && ret == 0) {
		vm_paths = build_paths(vm_desc);
		return (vm_paths ? 0 : 1);
	}

	memset(vm,0,sizeof(*vm));
	sprintf(vm->object,"%s:%s", mo_ref->type, mo_ref->value);
	vim_results2desc(s->vim,ret->nodes,vm_desc);
	fixname(vm->name);
	get_ci_name(s, vm);
	sprintf(temp,"%s:%s", host_obj.type, host_obj.value);
	vm->host = get_host(s, temp);
	if (!vm->host) {
		printf("error: get_vm_info: unable to get host (%s) for vm (%s)\n", temp, vm->object);
		return 1;
	}
	vm->farm = vm->host->farm;
	dprintf("farm name: %s\n", vm->farm->name);
	if (strcmp(vm->farm->name,"unknown") == 0) {
		set = get_result(ret->nodes,"parent");
		dprintf("set: %p\n", set);
		if (set) {
			struct farm *farm;
			sprintf(temp,"%s:%s", set->type, set->value);
			dprintf("calling get_farm\n");
			farm = get_farm(s,temp);
			if (farm) vm->farm = farm;
		}
	}
	dprintf("toolsStatus: %s\n", vm->tools);
	if (strncmp(vm->tools,"tools",5) == 0) {
		strcpy(vm->tools,&vm->tools[5]);
		dprintf("toolsStatus(2): %s\n", vm->tools);
	}
	if (!strlen(vm->tools)) strcpy(vm->tools,"unknown");

	//2015-08-27T03:09:52.755498Z
#ifdef BOOT_TIME
	vm->boot_time[10] = ' ';
	vm->boot_time[26] = 0;
//	bt[10] = ' ';
//	bt[26] = 0;
//	printf("name: %s, state: %s, boot_time: %s, bt: %s\n", vm->name, vm->state, vm->boot_time, bt);
#endif

	dprintf("annotation: %s\n", vm->annotation);
	{
		register char *p;
		for(p=vm->annotation; *p; p++) {
			if (*p == '\n' || *p == '\r' || *p == '"' || *p == '\'' || *p == '\\') *p = ' ';
		}
	}
	dprintf("annotation: %s\n", vm->annotation);

	vm->files = list_create();
	set = get_result(ret->nodes, "layoutEx.file");
//	dprintf("layoutEx.file set: %p\n",set);
	if (set) {
		unsigned long long size;

		size = 0;
	        dprintf("set->name: %s\n", set->name);
		list_reset(set->nodes);
		while((set2 = list_get_next(set->nodes)) != 0) {
			dprintf("set2->name: %s\n", set2->name);
			if (strcmp(set2->name,"VirtualMachineFileLayoutExFileInfo") != 0) continue;
				struct vim_res2desc vm_desc[] = {
				{ "name", VIM_TYPE_STRING, new_file.name, sizeof(new_file.name), 0, 1 },
				{ "type", VIM_TYPE_STRING, new_file.type, sizeof(new_file.type), 0, 1 },
				{ "size", VIM_TYPE_QUAD, &new_file.size, 0, 0, 0 },
				{ 0, 0, 0, 0, 0, 0 }
				};
				vim_results2desc(s->vim,set2->nodes,vm_desc);
				dprintf("name: %s\n", new_file.name);
				dprintf("type: %s\n", new_file.type);
				dprintf("size: %lld\n", new_file.size);
				list_add(vm->files,&new_file,sizeof(new_file));
				size += new_file.size;
#if 0

			p = get_result_value(set2->nodes, "size");
			if (!p) p = "";
			dprintf("p: %s\n", p);
			size += strtoll(p,0,10);
			dprintf("size: %lld\n", size);
#endif
		}
		dprintf("size: %lld\n", size);
		vm->size_total = size/1048576;
	}
	dprintf("size_total: %d\n", vm->size_total);

	vm->disks = list_create();
//	dprintf("config.hardware.device set: %p\n", set);
        set = get_result(ret->nodes, "config.hardware.device");
        if (set) {
//	        dprintf("set->name: %s\n", set->name);
		list_reset(set->nodes);
		while((set2 = list_get_next(set->nodes)) != 0) {
//			dprintf("set2->name: %s\n", set2->name);
			if (strcmp(set2->name,"VirtualDevice") != 0) continue;
			p = get_result_value(set2->nodes, "deviceInfo.label");
			if (!p) p = "";
//			dprintf("p: %s\n", p);
#define HDLABEL "Hard disk"
			if (strncmp(p,HDLABEL,strlen(HDLABEL)) == 0) {
				struct vim_res2desc vm_desc[] = {
				{ "deviceInfo.label", VIM_TYPE_STRING, new_disk.name, sizeof(new_disk.name), 0, 1 },
				{ "backing.fileName", VIM_TYPE_STRING, new_disk.filename, sizeof(new_disk.filename), 0, 1 },
				{ "capacityInKB", VIM_TYPE_QUAD, &new_disk.size, 0, 0, 0 },
				{ "backing.thinProvisioned", VIM_TYPE_STRING, new_disk.isthin, sizeof(new_disk.isthin), 0, 0 },
				{ 0, 0, 0, 0, 0, 0 }
				};
				vim_results2desc(s->vim,set2->nodes,vm_desc);
				dprintf("name: %s\n", new_disk.name);
				dprintf("filename: %s\n", new_disk.filename);
				dprintf("size: %lld\n", new_disk.size);
				dprintf("isthin: %s\n", new_disk.isthin);
				list_add(vm->disks,&new_disk,sizeof(new_disk));
#define NETLABEL "Network adapter"
			} else if ((strncmp(p,NETLABEL,strlen(NETLABEL)) == 0) && (strlen(vm->mac) == 0)) {
				struct vim_res2desc vm_desc[] = {
				{ "backing.deviceName", VIM_TYPE_STRING, vm->network, sizeof(vm->network), 0, 1 },
				{ "macAddress", VIM_TYPE_STRING, vm->mac, sizeof(vm->mac), 0, 1 },
				{ 0, 0, 0, 0, 0, 0 }
				};
				vim_results2desc(s->vim,set2->nodes,vm_desc);
				dprintf("network: %s\n", vm->network);
				dprintf("mac: %s\n", vm->mac);
			}
		}
	}

	return 0;
}

int init_vms(struct csession *s) {
	dprintf("calling get_vm_info...\n");
	return get_vm_info(s,0,0);
}

struct vm *get_vm(struct csession *s, char *object) {
	struct vm *vm, new_vm;
	list results;
	struct returnval *ret;
	struct ManagedObjectReference *mo_ref;

	dprintf("object: %s\n", object);

	list_reset(s->vms);
	while((vm = list_get_next(s->vms)) != 0) {
		if (strcmp(vm->object,object) == 0) {
			dprintf("found.\n");
			return vm;
		}
	}
	dprintf("not found.\n");

	mo_ref = str2mor(object);
	results = vim_getinfo(s->vim, mo_ref->type, vm_paths, 0, mo_ref);
	if (list_count(results) != 1) {
		printf("get_vm(%s): result count != 1!\n", object);
		return 0;
	}
	free_mor(mo_ref);
	ret = results->first->item;
	if (get_vm_info(s, &new_vm, ret))
		return 0;
	return add_vm(s->vms, &new_vm);
}

int get_vms(struct csession *s) {
	struct vm new_vm;
	list results;
	struct returnval *ret;

	/* Bulk request all VMs */
	results = vim_getinfo(s->vim, "VirtualMachine", vm_paths, 0, 0);
	dprintf("result count: %d\n", list_count(results));
	if (list_count(results) < 1) {
		/* XXX not an error */
		return 0;
	}

	/* Process request */
	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
		dprintf("----------------------------------------------------------------------\n");
		dprintf("VM Object: %s:%s\n", ret->obj.type, ret->obj.value);
		if (get_vm_info(s,&new_vm,ret)) continue;
#if 0
		{
			struct vm_disks_record *disk;
			list_reset(new_vm.disks);
			while((disk = list_get_next(new_vm.disks)) != 0) {
				dprintf(">>> NAME: %s\n", disk->name);
			}
		}
#endif
		add_vm(s->vms, &new_vm);
	}

	return 0;
}

static void vm2rec(struct vms_record *rec, struct vm *vm) {
//	memset(rec,0,sizeof(*rec));
	rec->farm_id = vm->farm->id;
	rec->host_id = vm->host->id;
	strncpy(rec->name, vm->name,sizeof(rec->name)-1);
	strncpy(rec->ci_name, vm->ci_name,sizeof(rec->ci_name)-1);
	strncpy(rec->uuid, vm->uuid,sizeof(rec->uuid)-1);
	strncpy(rec->os, vm->os, sizeof(rec->os));
	strncpy(rec->tools, vm->tools, sizeof(rec->tools)-1);
	rec->cpu_total = vm->cpu_total;
	rec->cpu_usage = vm->cpu_usage;
	rec->mem_total = vm->mem_total;
	rec->mem_usage = vm->mem_usage;
	rec->size_total = vm->size_total;
	strncpy(rec->state, vm->state, sizeof(rec->state)-1);
	strncpy(rec->network, vm->network, sizeof(rec->network)-1);
	strncpy(rec->ip, vm->ip, sizeof(rec->ip)-1);
	strncpy(rec->mac, vm->mac, sizeof(rec->mac)-1);
#ifdef BOOT_TIME
	strncpy(rec->boot_time, vm->boot_time, sizeof(rec->boot_time)-1);
#else
	strcpy(rec->boot_time,"1970-01-01 00:00:01");
#endif
	strncpy(rec->annotation, vm->annotation, sizeof(rec->annotation)-1);
}

#ifdef DO_PERF
static int get_vm_metrics(struct csession *s, struct vm *vm) {
	char query[4096], *beginTime;
	struct vim_perfmetric vm_metrics[] = {
		{ "usagemhz", 	"cpu", 	"megaHertz", 		"average" },
		{ "ready",	"cpu",	"millisecond",		"summation" },
		{ "consumed", 	"mem", 	"kiloBytes",		"average" },
		{ "usage", 	"disk", "kiloBytesPerSecond",	"average" },
		{ "usage", 	"net",	"kiloBytesPerSecond",	"average" },
		{ 0,0,0,0 }
	};
	struct ManagedObjectReference *mo_ref;
	int i, count, cpu, ready, mem, disk, net;

//	if (strcmp(vm->name,"g2t0979g") != 0) return 0;
	beginTime = 0;
	sprintf(query,"SELECT MAX(time) FROM vm_perf WHERE vm_id = %d",vm->id);
	if (db_exec(s->db,query) == 0) {
		query[0] = 0;
		if (db_fetch(s->db) == 0) {
			SQLRETURN ret;

			ret = SQLGetData(s->db->hstmt, 1, SQL_C_CHAR, (SQLPOINTER) query, sizeof(query), 0);
			dprintf("ret: %d\n", (int)ret);
			if (ret == 0) {
				query[10] = 'T';
				strcat(query,"Z");
				beginTime = query;
			}
		}
		db_fetch_done(s->db);
	}
//	if (beginTime) dprintf("beginTime: %s\n", beginTime);
	dprintf("getting metrics...\n");

	/* XXX this assumes all metrics have the same count ... tsk tsk tsk */
	mo_ref = str2mor(vm->object);
	if (vim_getperfmetrics(s->vim, mo_ref, beginTime, vm_metrics, 0) == 0) {
		count = vm_metrics[0].count;
		dprintf("count: %d\n", count);
		if (!count) {
			vim_getperfmetrics(s->vim, mo_ref, beginTime, vm_metrics, 1);
			count = vm_metrics[0].count;
			dprintf("count: %d\n", count);
		}
		for(i=0; i < count; i++) {
			if (!vm_metrics[0].data) continue;
			cpu = (vm_metrics[0].data ? vm_metrics[0].data[i].value : 0);
			ready = (vm_metrics[1].data ? vm_metrics[1].data[i].value : 0);
			mem = (vm_metrics[2].data ? vm_metrics[2].data[i].value : 0);
			disk = (vm_metrics[3].data ? vm_metrics[3].data[i].value : 0);
			net = (vm_metrics[4].data ? vm_metrics[4].data[i].value : 0);
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
				vm->id,						/* vm_id */
				cpu,ready,mem,disk,net
			);
			if (db_exec(s->db,query)) printf("bad insert: %s\n", query);
		}
	} else {
		printf("unable to get perf metrics for: %s\n", vm->name);
	}
	return 0;
}
#endif

void insert_hist_rec(struct csession *s,char *vm_name, int host_from, char *host_to) {
	struct hosts_record hosts_rec;
	struct vm_hist_record hist_rec;
	char from_name[64],clause[64];

	dprintf("vm_name: %s, host_from: %d, host_to: %s\n", vm_name,host_from,host_to);
	if (host_from) {
		sprintf(clause," WHERE id = %d", host_from);
		if (hosts_select_record(s->db,&hosts_rec,clause)) {
			printf("error: unable to get host_from name (id: %d)\n", host_from); 
			strcpy(from_name,"unknown");
		} else {
			strcpy(from_name,hosts_rec.name);
		}
	} else {
		strcpy(from_name,"none");
	}
	dprintf("vm_name: %s, host_from: %s, host_to: %s\n", vm_name,from_name,host_to);
	strcpy(hist_rec.vm_name,vm_name);
	strcpy(hist_rec.host_from,from_name);
	strcpy(hist_rec.host_to,host_to);
	vm_hist_insert(s->db,&hist_rec);
}

int update_vms(struct csession *s) {
	struct vms_record rec;
	struct vm *vm;
	struct vm_files_record *file;
	struct vm_disks_record *disk;
	char clause[256];
	int do_byname,do_update;

	list_reset(s->vms);
	while((vm = list_get_next(s->vms)) != 0) {
		dprintf("vm: %p\n", vm);
		dprintf("vm: %s\n", vm->name);
		dprintf("server: %s, farm: %s, host: %s, name: %s, uuid: %s\n",
			vm->farm->server, vm->farm->name, vm->host->name, vm->name, vm->uuid);
		do_byname = do_update = 0;
		/* XXX first, select by UUID */
		if (strlen(vm->uuid)) {
			sprintf(clause," WHERE uuid = '%s'", vm->uuid);
			if (vms_select_record(s->db,&rec,clause)) {
				dprintf("==> uuid not found, trying farm_id and name...\n");
				do_byname = 1;
			} else {
				do_update = 1;
			}
		} else {
			do_byname = 1;
		}
		dprintf("vm: %p\n", vm);
		dprintf("do_byname: %d\n", do_byname);
		if (do_byname) {
			sprintf(clause," WHERE farm_id = %d and name = '%s'", vm->farm->id, vm->name);
			if (vms_select_record(s->db,&rec,clause)) {
				dprintf("==> vm not found, inserting new record...\n");
				vm2rec(&rec,vm);
				if (vms_insert(s->db,&rec)) return 1;
				vms_select_record(s->db,&rec,clause);
				insert_hist_rec(s,vm->name,0,vm->host->name);
			} else {
				do_update = 1;
			}
		}
		dprintf("vm: %p\n", vm);
		dprintf("do_update: %d\n", do_update);
		if (do_update) {
			dprintf("vm->name: %s, rec.host_id: %d, vm->host->id: %d\n", vm->name, rec.host_id, vm->host->id);
			if (rec.host_id && rec.host_id != vm->host->id) {
				dprintf("==> host_ids dont match, adding hist rec...\n");
				insert_hist_rec(s,vm->name,rec.host_id,vm->host->name);
			}
			vm2rec(&rec,vm);
		}
		sprintf(clause,",last_seen = CURRENT_TIMESTAMP WHERE id = %d", rec.id);
		if (vms_update_record(s->db,&rec,clause)) return 1;
		dprintf("vm: %p\n", vm);
		vm->id = rec.id;
		sprintf(clause,"WHERE vm_id = %d", vm->id);

		vm_files_delete(s->db,clause);
		list_reset(vm->files);
		while((file = list_get_next(vm->files)) != 0) {
			file->vm_id = vm->id;
			vm_files_insert(s->db,file);
		}

		vm_disks_delete(s->db,clause);
		list_reset(vm->disks);
		while((disk = list_get_next(vm->disks)) != 0) {
			disk->vm_id = vm->id;
			vm_disks_insert(s->db,disk);
		}
#ifdef DO_PERF
		get_vm_metrics(s,vm);
#endif
        }
	dprintf("returning!\n");
	return 0;
}
