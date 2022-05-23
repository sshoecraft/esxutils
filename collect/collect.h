
#ifndef __COLLECT_H
#define __COLLECT_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vim.h"
#include "util.h"
#include "db.h"

#include "soapfuncs/ServiceContent.h"
#include "soapfuncs/RetrieveProperties.h"

struct csession {
	struct vim_session *vim;
	list farms,hosts,datastores,vms;
	char addr[16];
	char query[4096];
	DB *db,*cidb;
};

struct farm {
	char object[64];				/* Object ID in vc */
	int id;						/* Record ID in db */
	char server[64];
	char name[64];
	int site_id;
};

struct host {
	char object[64];
	int id;
	struct farm *farm;
	char name[64];
	char uuid[64];
	char os[16];
	char version[8];
	int build;
	char model[32];
	char serial[32];
	char bios[16];
	char status[16];
	float psp;
	char cpu_model[64];
	int cpu_pkgs;
	int cpu_count;
	int cpu_speed;
	int cpu_total;
	int mem_total;
	int cons_mem;
	char in_maint[8];
	char subnet[16];
	list datastores;				/* list of datastores */
	list alarms;					/* list of char ptrs */
	char state[16];
};

struct datastore {
	char object[64];
	int id;
	char uuid[128];
	char name[64];
	int blocksize;
	int total;
	int free;
};

struct host_datastore {
	int host_id;
	int datastore_id;
};

struct vm {
	char object[64];
	int id;
	char name[64];
	char ci_name[64];
	char uuid[64];
	struct farm *farm;
	struct host *host;
	char os[32];
	char tools[16];
	char state[32];
	char network[32];
	char ip[16];
	char mac[24];
	int cpu_total;
	int cpu_usage;
	int mem_total;
	int mem_usage;
	int size_total;
	char boot_time[32];
	list files;
	list disks;
	char annotation[1024];
};

struct recdesc {
	int index;
	int type;
	void *ptr;
};

enum RD_TYPES {
	RD_INT,
	RD_STRING,
	RD_FLOAT,
};

int collect(char *, int, char *, char *);

int init_farms(struct csession *);
struct farm *get_farm(struct csession *, char *);
int get_farms(struct csession *);
int update_farms(struct csession *);

int init_hosts(struct csession *);
struct host *get_host(struct csession *, char *);
int get_hosts(struct csession *);
int update_hosts(struct csession *);

int init_vms(struct csession *);
struct vm *get_vm(struct csession *s, char *);
int get_vms(struct csession *);
int update_vms(struct csession *);

int init_datastores(struct csession *);
struct datastore *get_datastore(struct csession *, char *);
int get_datastores(struct csession *);
int update_datastores(struct csession *);

int update_host_datastores(struct csession *s);

int get_alarms(struct csession *, struct host *);
int sync_alarms(struct csession *, struct host *);

#if 0
int cmpmor(struct ManagedObjectReference *,struct ManagedObjectReference *);
char **build_paths(struct vim_res2desc *);
struct ManagedObjectReference *alloc_mor(int,int);
struct ManagedObjectReference *str2mor(char *);
struct ManagedObjectReference *set2mor(struct propSet *);
void free_mor(struct ManagedObjectReference *);
void fixstr(char *);
#endif

#include "db.h"

//		printf("object: type: %s, value: %s\n", mo_ref->type, mo_ref->value); 
/*
	if (!p) { \
		printf("%s: unable to get %s\n",__FUNCTION__,str); \
		return 1; \
	}
*/
#define GETVAL(str) p = get_result_value(ret->nodes,str);

extern char *actions_file;
void log_action(char *,...);
extern int do_update;
extern int do_mksnap;

#endif
