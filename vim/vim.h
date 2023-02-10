
#ifndef __VIM_H
#define __VIM_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>	/* req'd for time_t */
#ifdef __WINDOWS__
#include <windows.h>
#endif
#include "list.h"

#ifndef _cplusplus
#ifndef _BOOL_DEFINED
typedef char bool;
#define _BOOL_DEFINED 1
#endif
#endif

/* Req'd for session */
#include "soapfuncs/ServiceContent.h"
#include "soapfuncs/UserSession.h"

/* Types */
enum VIM_TYPE {
	VIM_TYPE_UNK,
	VIM_TYPE_OBJ,
	VIM_TYPE_POBJ,	
	VIM_TYPE_STRING,
	VIM_TYPE_PSTRING,
	VIM_TYPE_BYTE,
	VIM_TYPE_SHORT,
	VIM_TYPE_INT,
	VIM_TYPE_QUAD,
	VIM_TYPE_FLOAT,
	VIM_TYPE_DOUBLE,
	VIM_TYPE_DATETIME,
	VIM_TYPE_BOOL,
	VIM_TYPE_FUNC,			/* special */
};

#if 0
struct ElementDescription {
	char *label;
	char *summary;
	char *key;
};

struct PerfCounterInfo {
	list associatedCounterIds;
	struct ElementDescription groupInfo;
	int key;
	int level;
	struct ElementDescription nameInfo;
	char *rollupType;
	char *statsType;
	struct ElementDescription unitInfo;
};

struct PerfMetricId {
	int counterId;			/* counterId */
	char *instance;			/* instance */
};
#endif

#ifndef __MOR_DEFINED
struct ManagedObjectReference {
	char *value;
	char *type;
};
#define __MOR_DEFINED 1
#endif

struct vim_apmetric {			/* AvailPerfMetric */
	char *type;			/* MOR type */
	list results;			/* ap results */
};

struct vim_session {
	struct soap *soap;
	char endpoint[128];
	char server[64];		/* Server name */
	int port;

	struct ServiceContent *sc;	/* Filled by vim_session */
	struct UserSession *info;	/* Filled by vim_login */
	list metrics;			/* Filled by vim_getallperfmetrics */
#ifdef CACHE_AP
	list apmetrics;			/* Filled by vim_getallperfmetrics */
#endif

	void *user;			/* app-controlled scratch */
};

struct vim_res2desc {
	char *path;
	int type;
	void *dest;
	int dlen;
	char *def;
	int req;
};

struct vim_perfdata {
	char *timeStamp;
	int value;
};

#define dump_metric(l,m) dprintf("%s: id: %d, name: %s, group: %s, unit: %s, type: %s\n", l, (m)->id, (m)->name, (m)->group, (m)->unit, (m)->type);

struct vim_perfmetric {
	char *name;
	char *group;
	char *unit;
	char *type;
	int id;				/* Filled in by vim_getperfdata */
//	char *instance;
	int count;			/* Filled in by vim_getperfdata */
	struct vim_perfdata *data;	/* Filled in by vim_getperfdata */
};

#if 0
struct LocalizedMethodFault {
	struct DynamicData *fault;
	char *localizedMessage;
};

struct ReconfigureClusterRequest {
	struct ManagedObjectReference *cluster;
	struct ClusterConfigSpec *spec;
	bool modify;
};

extern char _vim_errorstr[1024];
#endif

/* Forward decs */
struct perfInfo;
struct ClusterConfigSpec;

int vim_getpass(char *,int);
struct vim_session *vim_connect(char *,int);
void vim_disconnect(struct vim_session *);
void vim_error(char *format,...);
int vim_login(struct vim_session *, char *, char *);
int vim_secure_login(struct vim_session *, char *, char *);
int vim_logout(struct vim_session *);
int vim_getvms(struct vim_session *s);
list vim_getinfo(struct vim_session *s, char *, char **, int all, struct ManagedObjectReference *);
struct ManagedObjectReference *vim_findobj(struct vim_session *vim, char *type, char *name);
struct propSet *get_result(list, char *);
char *get_result_value(list, char *);
int match(char *,char *);
int vim_results2desc(struct vim_session *, list, struct vim_res2desc *);
list vim_gettasks(struct vim_session *, struct ManagedObjectReference *, list);
list vim_gettasksbystate(struct vim_session *, struct ManagedObjectReference *, char *);
struct TaskInfo *vim_findtask(list,char *);
int vim_wait4task(struct vim_session *,struct ManagedObjectReference *,int);
int vim_reconnect_host(struct vim_session *,struct ManagedObjectReference *,char *,int,char *,char *, char *);
int vim_disconnect_host(struct vim_session *,struct ManagedObjectReference *,char *);
struct ManagedObjectReference *vim_add_host(struct vim_session *vim, struct ManagedObjectReference *cluster, char *hostName, int, char *userName, char *password, char *license, char *);
struct ManagedObjectReference *vim_createdc(struct vim_session *,char *);
struct ManagedObjectReference *vim_createcluster(struct vim_session *, struct ManagedObjectReference *, char *, struct ClusterConfigSpec *);
int vim_entermm(struct vim_session *vim, struct ManagedObjectReference *mo_ref, int);
int vim_exitmm(struct vim_session *vim, struct ManagedObjectReference *mo_ref, int);
int vim_ismm(struct vim_session *vim, struct ManagedObjectReference *mo_ref);
int vim_isconnected(struct vim_session *vim, struct ManagedObjectReference *mo_ref);
int vim_getperfmetrics(struct vim_session *s, struct ManagedObjectReference *mor, char *startTime, struct vim_perfmetric *,int);
//int vim_checkperfsupport(struct vim_session *s, struct ManagedObjectReference *mor);
int vim_destroy(struct vim_session *s, struct ManagedObjectReference *mor);
int vim_poweroff(struct vim_session *s, struct ManagedObjectReference *mor);
int vim_rename(struct vim_session *s, struct ManagedObjectReference *mo_ref, char *newname);
int vim_getallperfmetrics(struct vim_session *s);
char **build_paths(struct vim_res2desc *desc);
int get_vcserver(char *version, char *cluster, char *host, char *server, int port, char *user);
int get_vcuser(char *server, int port, char *user);

/* MOR utils */
struct ManagedObjectReference *alloc_mor(int ts, int vs);
struct ManagedObjectReference *str2mor(char *in_object);
struct ManagedObjectReference *set2mor(struct propSet *set);
void free_mor(struct ManagedObjectReference *mo_ref);

/* Cred store */
int vim_cred_add(int,char *,char *,char *);
int vim_cred_del(int,char *,char *);
char *vim_cred_get(char *,char *);

#ifndef dprintf
#ifdef DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(format, args...) /* noop */
#endif /* !DEBUG */
#endif

#endif
