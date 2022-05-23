
#include "ivim.h"
#include "AddHost.h"
#include "ReconnectHost.h"
#include "DisconnectHost.h"
#include "QueryHostConnection.h"
#include "RetrieveProperties.h"

int vim_isconnected(struct vim_session *vim, struct ManagedObjectReference *mo_ref) {
//	struct HostConnectInfo info;
	char *paths[] = { "runtime.connectionState", 0 }, *p;
	int count;
	struct returnval *ret;
	list results;

	dprintf("mo_ref: %s:%s\n", mo_ref->type, mo_ref->value);
//	dprintf("calling QueryHostConnectionInfo...\n");
//	QueryHostConnectionInfo(vim->soap,vim->endpoint,mo_ref,&info);

	results = vim_getinfo(vim, mo_ref->type, paths, 0, mo_ref);
	count = list_count(results);
//	dprintf("count: %d\n", count);
	if (count != 1) return -1;
	ret = results->first->item;
//	dprintf("ret: %p\n", ret);
	if (!ret) return -1;
	p = get_result_value(ret->nodes, "runtime.connectionState");
//	dprintf("p: %p\n", p);
	if (!p) return -1;
	dprintf("p: %s\n", p);
	return (strcmp(p,"connected") == 0 ? 1 : 0);
}

struct ManagedObjectReference *vim_add_host(struct vim_session *vim, struct ManagedObjectReference *cluster, char *hostName, int port, char *userName, char *password, char *license, char *tp) {
	struct AddHostRequest req;
	struct ManagedObjectReference *host, *task;
	struct HostConnectSpec spec;
	int r;

	dprintf("cluster: %p, hostName: %s, port: %d, userName: %s, password: %s, license: %s, tp: %s\n", cluster, hostName, port, userName, password, license, tp);

	dprintf("setting spec...\n");
	memset(&spec,0,sizeof(spec));
	spec.hostName = hostName;
	spec.port = (port ? port : 443);
	spec.sslThumbprint = tp;
	spec.userName = userName;
	spec.password = password;
	spec.force = 1;

	dprintf("setting req...\n");
	memset(&req,0,sizeof(req));
	req.cluster = cluster;
	req.spec = &spec;
	req.asConnected = 1;
	req.license = license;

	dprintf("calling AddHost...\n");
	r = AddHost(vim->soap, vim->endpoint, &req, &task);
	dprintf("r: %d\n", r);
	if (r) return 0;

	if (vim_wait4task(vim,task,0)) return 0;

	host = vim_findobj(vim,"HostSystem",hostName);
	return host;
}

int vim_reconnect_host(struct vim_session *vim, struct ManagedObjectReference *mo_ref, char *hostName, int port, char *userName, char *password, char *tp) {
	struct ReconnectHostRequest req;
	struct ManagedObjectReference *host_obj, *task;
	struct HostConnectSpec spec;
	int r;

	dprintf("mo_ref: %p, hostName: %s, port: %d, userName: %s, password: %s, tp: %s\n", mo_ref, hostName, port, userName, password, tp);

	if (!mo_ref) {
		if (!hostName) {
			printf("vim_reconnect_host: both mo_ref and hostName are null!\n");
			return 1;
		}
		host_obj = vim_findobj(vim, "HostSystem", hostName);
		if (!host_obj) {
			printf("vim_reconnect_host: unable to find host: %s\n", hostName);
			return 1;
		}
	} else {
		host_obj = mo_ref;
	}
	host_obj = mo_ref;
	dprintf("host_obj: %p\n", host_obj);

	dprintf("setting spec...\n");
	memset(&spec,0,sizeof(spec));
	spec.hostName = hostName;
	spec.port = (port ? port : 443);
	spec.sslThumbprint = tp;
	spec.userName = userName;
	spec.password = password;
	spec.force = 1;

	dprintf("setting req...\n");
	memset(&req,0,sizeof(req));
	req.host = host_obj;
	req.cnxSpec = &spec;

	dprintf("calling Reconnect...\n");
	r = ReconnectHost(vim->soap, vim->endpoint, &req, &task);
	dprintf("r: %d\n", r);
	if (r) return 1;

	return vim_wait4task(vim,task,0);
}

int vim_disconnect_host(struct vim_session *vim, struct ManagedObjectReference *mo_ref, char *hostName) {
	struct DisconnectHostRequest req;
	struct ManagedObjectReference *host_obj, *task;
	int r;

	dprintf("mo_ref: %p, hostName: %s\n", mo_ref, hostName);

	if (!mo_ref) {
		host_obj = vim_findobj(vim, "HostSystem", hostName);
		if (!host_obj) {
			printf("vim_disconnect_host: unable to find host: %s\n", hostName);
			return 1;
		}
	} else {
		host_obj = mo_ref;
	}
	dprintf("host_obj: %p\n", host_obj);

	dprintf("setting req...\n");
	memset(&req,0,sizeof(req));
	req.host = host_obj;

	dprintf("calling Reconnect...\n");
	r = DisconnectHost(vim->soap, vim->endpoint, &req, &task);
	dprintf("r: %d\n", r);
	if (r) return 1;

	return vim_wait4task(vim,task,0);
}
