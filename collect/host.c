
//#define DEBUG 1
#include "collect.h"
#include "gendb_hosts.h"
#include "gendb_datastores.h"

static char **host_paths;

static struct host *add_host(list hosts, struct host *new_host) {
	struct host *host;

	list_reset(hosts);
	while((host = list_get_next(hosts)) != 0) {
		if (strcmp(host->object,new_host->object) == 0) {
			return host;
		}
	}
	dprintf("adding host: %s\n", new_host->name);
	host = list_add(hosts, new_host, sizeof(*new_host));

	return host;
}

#define dump_set(l,s) dprintf("%s: name: %s, type: %s, value: %s, parent: %s\n", l, (s)->name, (s)->type, (s)->value, ((s)->parent ? s->parent->name : "none"));

char *get_subnet(struct propSet *net) {
	struct propSet *set;
//	struct propSet *set2;

	list_reset(net->nodes);
	while((set = list_get_next(net->nodes)) != 0) {
		dump_set("set",set);
	}
	exit(0);
}

int get_host_info(struct csession *s, struct host *host, struct returnval *ret) {
	struct propSet *set, *set2;
	struct datastore *ds;
	char temp[128],*p;
	long long mem_size,cons_mem;
	struct vim_res2desc host_desc[] = {
		{ "parent", VIM_TYPE_UNK, 0, 0, 0, 1 },
		{ "datastore", VIM_TYPE_UNK, 0, 0, 0, 1 },
		{ "summary.config.name", VIM_TYPE_STRING, host->name, sizeof(host->name)-1, 0, 1 },
		{ "summary.hardware.model", VIM_TYPE_STRING, host->model, sizeof(host->model)-1, "unknown", 0 },
		{ "overallStatus", VIM_TYPE_STRING, host->status, sizeof(host->status)-1, "brown", 0 },
		{ "summary.config.product.name", 0, 0, 0, 0, 0 },
		{ "summary.config.product.version", VIM_TYPE_STRING, host->version, sizeof(host->version)-1, "0.0.0", 0 },
		{ "summary.config.product.build", VIM_TYPE_INT, &host->build, 0, 0, 0 },
		{ "summary.hardware.numCpuPkgs", VIM_TYPE_INT, &host->cpu_pkgs, 0, 0, 0 },
		{ "summary.hardware.numCpuCores", VIM_TYPE_INT, &host->cpu_count, 0, 0, 0 },
		{ "summary.hardware.cpuMhz", VIM_TYPE_INT, &host->cpu_speed, 0, 0, 0 },
		{ "summary.hardware.cpuModel", VIM_TYPE_STRING, host->cpu_model, sizeof(host->cpu_model)-1, "unknown", 0 },

		{ "summary.hardware.memorySize", VIM_TYPE_QUAD, &mem_size, 0, 0, 0 },
		{ "summary.hardware.otherIdentifyingInfo", 0, 0, 0, 0, 0 },
		{ "summary.hardware.uuid", VIM_TYPE_STRING, host->uuid, sizeof(host->uuid)-1, 0, 0 },
		{ "config.consoleReservation.serviceConsoleReserved", VIM_TYPE_QUAD, &cons_mem, 0, 0, 0 },
		{ "summary.runtime.inMaintenanceMode", VIM_TYPE_STRING, &host->in_maint, sizeof(host->in_maint)-1, 0, 0 },
		{ "summary.runtime.connectionState", VIM_TYPE_STRING, &host->state, sizeof(host->state)-1, 0, 0 },
		{ "hardware.biosInfo.releaseDate", 0, 0, 0, 0, 0 },
		{ "config.network", 0, 0, 0, 0, 0 },
		{ 0, 0, 0, 0, 0, 0 }
	};

	/* Build paths? */
	if (s == 0 && host == 0 && ret == 0) {
		host_paths = build_paths(host_desc);
		return (host_paths ? 0 : 1);
	}

	temp[0] = 0;
	memset(host,0,sizeof(*host));
	sprintf(host->object,"%s:%s", ret->obj.type, ret->obj.value);
	vim_results2desc(s->vim,ret->nodes, host_desc);
	if (!strlen(host->name)) strcpy(host->name,"unknown");
	if (mem_size) host->mem_total = mem_size / 1024;
	if (cons_mem > 1048576) host->cons_mem = cons_mem / 1048576;
	trim(host->cpu_model);
	host->cpu_total = host->cpu_count * host->cpu_speed;

	/* OS */
	p = get_result_value(ret->nodes,"summary.config.product.name");
	if (p) strcpy(host->os,strele(1," ",p));
#if 0
	if (!strlen(host->os)) {
		dprintf("warning: unable to get OS for %s.  Setting to: 'ESXi'\n", host->name);
	}
#endif
	dprintf("os: %s\n", host->os);

	/* BIOS */
	p = get_result_value(ret->nodes,"hardware.biosInfo.releaseDate");
	if (p) {
		char *p2;

//		dprintf("biosInfo.releaseDate: %s\n", p);
		p2 = strchr(p,'T');
		if (p2) *p2 = 0;
		dprintf("biosInfo.releaseDate: %s\n", p);
		strcpy(host->bios,p);
	}
	dprintf("bios: %s\n", host->bios);

	/* Display low cons mem for ESX (not i) */
	if (strcmp(host->os,"ESX") == 0 && host->cons_mem < 800) printf("WARNING: low console memory! name: %s, cons_mem: %lld\n", host->name, cons_mem);

	/* Alarms */
	host->alarms = list_create();
	dprintf("overallStatus: %s\n", host->status);
	if (strcmp(host->status,"green") != 0) get_alarms(s, host);

	/* Get farm */
	set = get_result(ret->nodes,"parent");
	if (!set) {
		dprintf("get_hosts: no parent found for obj: %s:%s\n", ret->obj.type, ret->obj.value);
//		sprintf(temp,"%s:%s", s->vim->sc->rootFolder->type, s->vim->sc->rootFolder->value);
		sprintf(temp,"0:0");
	} else {
		sprintf(temp,"%s:%s", set->type, set->value);
	}
	dprintf("calling get_farm\n");
	host->farm = get_farm(s,temp);
	if (!host->farm) {
		dprintf("error getting farm!\n");
		exit(1);
	}

	/* Get datastores */
	host->datastores = list_create();
	set = get_result(ret->nodes, "datastore");
	if (set) {
		list_reset(set->nodes);
		while((set2 = list_get_next(set->nodes)) != 0) {
			dprintf("set2->type: %s, set2->value: %s\n", set2->type, set2->value);
			sprintf(temp,"%s:%s", set2->type, set2->value);
			ds = get_datastore(s, temp);
			if (!ds) {
				printf("get_host_info: unable to get datastore for obj %s\n", temp);
				return 1;
			}
			list_add(host->datastores, ds, 0);
		}
	}

	/* Serial */
	set = get_result(ret->nodes, "summary.hardware.otherIdentifyingInfo");
//	dprintf("set: %p\n", set);
	if (set) {
		struct propSet *set3;
		char tmp_serial[32];
		int done;

		list_reset(set->nodes);
		while((set2 = list_get_next(set->nodes)) != 0) {
//			dprintf("name: %s, set2->type: %s, set2->value: %s, nodes: %p\n", set2->name, set2->type, set2->value, set2->nodes);

			done = 0;
			list_reset(set2->nodes);
			while((set3 = list_get_next(set2->nodes)) != 0) {
//				dprintf("set3: name: %s, type: %s, value: %s, nodes: %p\n", set3->name, set3->type, set3->value, set3->nodes);

				if (strcmp(set3->name,"identifierValue") == 0) {
					tmp_serial[0] = 0;
					strncpy(tmp_serial,set3->value,sizeof(tmp_serial)-1);
				} else if (strcmp(set3->name,"identifierType") == 0) {
					char *p = get_result_value(set3->nodes,"label");
					if (p) {
//						dprintf("p: %s\n", p);
						if (strcmp(p,"Service tag") == 0) {
							strcpy(host->serial,tmp_serial);
							done = 1;
							break;
						}
					}
				}
			}
			if (done) break;
		}
		if (strncmp(host->serial,"VMware",6) == 0) host->serial[0] = 0;
		dprintf("host->serial: %s\n", host->serial);
	}

	/* Get subnet */
#if 0
//	ip = get_result_value(ret->nodes,"config.network.consoleVnic.HostVirtualNic.spec.ip.ipAddress");
	ip = get_network(get_result(ret->nodes,"config.network"));
	dprintf("ip: %s\n", (ip ? ip : "null"));
	if (ip) {
		mask = get_result_value(ret->nodes,"config.network.consoleVnic.HostVirtualNic.spec.ip.subnetMask");
		dprintf("mask: %s\n", (mask ? mask : "null"));
		if (mask) {
			char datafile[64];
			FILE *fp;

			sprintf(datafile,"/var/tmp/collect.%d",getpid());
			sprintf(temp,"/bin/ipcalc --network %s %s | /bin/awk -F= '{ print $2 }' > %s 2>&1",ip,mask,datafile);
			dprintf("temp: %s\n", temp);
			system(temp);
			fp = fopen(datafile,"r");
			if (fp) {
				fgets(temp,sizeof(temp)-1,fp);
				if (strlen(temp)) temp[strlen(temp)-1] = 0;
				host->subnet[0] = 0;
				strncpy(host->subnet, temp, sizeof(host->subnet)-1);
				dprintf("subnet: %s\n", host->subnet);
				fclose(fp);
			}
			unlink(datafile);
		}
	}
#endif

	return 0;
}

int init_hosts(struct csession *s) {
	return get_host_info(0,0,0);
}

struct host *get_host(struct csession *s, char *object) {
	struct host *host, new_host;
	list results;
	struct returnval *ret;
	struct ManagedObjectReference *mo_ref;

	dprintf("object: %s\n", object);

	list_reset(s->hosts);
	while((host = list_get_next(s->hosts)) != 0) {
		if (strcmp(host->object,object) == 0) {
			dprintf("found.\n");
			return host;
		}
	}
	dprintf("not found.\n");

	mo_ref = str2mor(object);
	results = vim_getinfo(s->vim, mo_ref->type, host_paths, 0, mo_ref);
	if (list_count(results) != 1) {
		printf("get_host(%s): result count != 1!\n", object);
		return 0;
	}
	free_mor(mo_ref);
	ret = results->first->item;
	if (get_host_info(s,&new_host, ret))
		return 0;
	return add_host(s->hosts, &new_host);
}

int get_hosts(struct csession *s) {
	struct host new_host;
	list results;
	struct returnval *ret;
#if 0
	char name[256], *state;
#endif
	register char *p;

	results = vim_getinfo(s->vim, "HostSystem", host_paths, 0, 0);
	dprintf("result count: %d\n", list_count(results));
	if (list_count(results) < 1) {
		/* XXX not an error */
		return 0;
	}

	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
		p = get_result_value(ret->nodes,"summary.config.name");
		if (!p) {
			printf("error: unable to get name for host obj: %s:%s!\n", ret->obj.type, ret->obj.value);
			continue;
		}

#if 0
		/* Skip disconnected hosts */
		state = get_result_value(ret->nodes,"summary.runtime.connectionState");
		if (!state) {
			state = get_result_value(ret->nodes,"summary.config.name");
			printf("error: unable to get connectionState! name: %s\n", (state ? state : "unknown"));
			exit(1);
		}
		if (!state) state = "";
//		printf("state: %s\n", state);
		if (strcmp(state,"disconnected") == 0) continue;

		{
			char *p = get_result_value(ret->nodes,"summary.config.name");
			if (p) printf("NAME: %s\n", p);
		}
#endif

		/* Get info */
		if (get_host_info(s,&new_host,ret))
			return 1;
		add_host(s->hosts,&new_host);
	}
	return 0;
}

void host2rec(struct hosts_record *rec, struct host *host) {
//	memset(rec,0,sizeof(*rec));
	rec->farm_id = host->farm->id;
	strncpy(rec->name, host->name, sizeof(rec->name)-1);
	strncpy(rec->uuid, host->uuid, sizeof(rec->uuid)-1);
	strncpy(rec->os, host->os, sizeof(rec->os)-1);
	strncpy(rec->version, host->version, sizeof(rec->version)-1);
	rec->build = host->build;
	strncpy(rec->model, host->model, sizeof(rec->model)-1);
	strncpy(rec->serial, host->serial, sizeof(rec->serial)-1);
	strncpy(rec->bios, host->bios, sizeof(rec->bios)-1);
	strncpy(rec->status, host->status, sizeof(rec->status)-1);
	rec->psp = host->psp;
	strncpy(rec->cpu_model, host->cpu_model, sizeof(rec->cpu_model)-1);
	rec->cpu_pkgs = host->cpu_pkgs;
	rec->cpu_count = host->cpu_count;
	rec->cpu_speed = host->cpu_speed;
	rec->cpu_total = host->cpu_total;
	rec->mem_total = host->mem_total;
	strncpy(rec->in_maint,host->in_maint, sizeof(rec->in_maint)-1);
	strncpy(rec->subnet,host->subnet, sizeof(rec->subnet)-1);
	rec->cons_mem = host->cons_mem;
	strncpy(rec->state,host->state,sizeof(rec->state)-1);
}

#ifdef DO_PERF
static int get_host_metrics(struct csession *s, struct host *host) {
	char query[4096], *beginTime;
	struct vim_perfmetric host_metrics[] = {
		{ "usagemhz", 	"cpu", 	"megaHertz", 		"average" },
		{ "consumed", 	"mem", 	"kiloBytes",		"average" },
		{ "usage", 	"disk", "kiloBytesPerSecond",	"average" },
		{ "usage", 	"net",	"kiloBytesPerSecond",	"average" },
		{ 0,0,0,0 }
	};
	struct ManagedObjectReference *mo_ref;
	int i, count, cpu, mem, disk, net;

	beginTime = 0;
	sprintf(query,"SELECT MAX(time) FROM host_perf WHERE host_id = %d",host->id);
	if (db_exec(s->db,query) == 0) {
		if (db_fetch(s->db) == 0) {
			SQLGetData(s->db->hstmt, 1, SQL_C_CHAR, (SQLPOINTER) query, sizeof(query), 0);
			query[10] = 'T';
			strcat(query,"Z");
			beginTime = query;
		}
		db_fetch_done(s->db);
	}
//	if (beginTime) dprintf("beginTime: %s\n", beginTime);

	dprintf("getting metrics...\n");

	/* XXX this assumes all metrics have the same count ... tsk tsk tsk */
	mo_ref = str2mor(host->object);
	if (vim_getperfmetrics(s->vim, mo_ref, beginTime, host_metrics, 0) == 0) {
		count = host_metrics[0].count;
		dprintf("count: %d\n", count);
		if (!count) {
			vim_getperfmetrics(s->vim, mo_ref, beginTime, host_metrics, 1);
			count = host_metrics[0].count;
			dprintf("count: %d\n", count);
		}
		for(i=0; i < count; i++) {
			if (!host_metrics[0].data) continue;
			cpu = (host_metrics[0].data ? host_metrics[0].data[i].value : 0);
			mem = (host_metrics[1].data ? host_metrics[1].data[i].value : 0);
			disk = (host_metrics[2].data ? host_metrics[2].data[i].value : 0);
			net = (host_metrics[3].data ? host_metrics[3].data[i].value : 0);
			if (cpu == 0 && mem == 0 && disk == 0 && net == 0) continue;
			sprintf(query,"INSERT INTO host_perf (time,host_id,cpu,mem,disk,net) VALUES (\"%s\",%d,%d,%d,%d,%d)",
				host_metrics[0].data[i].timeStamp,		/* time (from cpu) */
				host->id,					/* host_id */
				cpu,mem,disk,net
			);
			if (db_exec(s->db,query)) printf("bad insert: %s\n",query);
		}
	}
	return 0;
}
#endif

int update_hosts(struct csession *s) {
	struct hosts_record rec;
	struct host *host;
	char clause[256];
	int do_byname,do_update;

	list_reset(s->hosts);
	while((host = list_get_next(s->hosts)) != 0) {
		dprintf("host: %s\n", host->name);
		do_byname = do_update = 0;
		if (strlen(host->uuid)) {
			sprintf(clause," WHERE uuid = '%s'", host->uuid);
			if (hosts_select_record(s->db, &rec, clause)) {
				do_byname = 1;
			} else {
				do_update = 1;
			}
		} else {
			do_byname = 1;
		}
		dprintf("do_byname: %d\n", do_byname);
		if (do_byname) {
			sprintf(clause," WHERE name = '%s'", host->name);
			if (hosts_select_record(s->db, &rec, clause)) {
				host2rec(&rec,host);
				if (hosts_insert(s->db,&rec)) return 1;
				hosts_select_record(s->db,&rec,clause);
			} else {
				do_update = 1;
			}
		}
		dprintf("do_update: %d\n", do_update);
		if (do_update) {
			dprintf("host->farm: %p\n", host->farm);
			if (!host->farm) {
				printf("farm is null for host %s!\n", host->name);
				exit(1);
			}
			/* XXX always update host rec */
			host2rec(&rec,host);
		}
		sprintf(clause,",last_seen = CURRENT_TIMESTAMP WHERE id = %d", rec.id);
		if (hosts_update_record(s->db,&rec,clause)) return 1;
		host->id = rec.id;
//		sync_alarms(s, host);
#ifdef DO_PERF
		get_host_metrics(s,host);
#endif
        }

	return 0;
}
