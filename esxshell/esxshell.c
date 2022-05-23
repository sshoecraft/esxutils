
#include <vim.h>

#include "soapfuncs/ManagedObjectReference.h"
#include "soapfuncs/RetrieveProperties.h"
#include "soapfuncs/Service.h"

#define VIM_CONNECT     5
#define VIM_LOGIN       6
#define VIM_NOT_FOUND   7

#define dumpmor(obj) printf("type: %s, val: %s\n", (obj)->type, (obj)->value);

static struct ManagedObjectReference *getserviceSystem(struct vim_session *vim) {
	char *paths[] = { "configManager", 0 };
	int count;
	list res;
	struct returnval *ret;
	struct propSet *set;
	struct ManagedObjectReference *ss;

	res = vim_getinfo(vim,"HostSystem",paths,0,0);
	count = list_count(res);
	dprintf("count: %d\n", count);
	if (count < 1) {
		printf("getserviceSystem: unable to get HostSystem!\n");
		return 0;
	}
	ret = res->first->item;
	set = get_result(ret->nodes,"configManager");
	if (set) {
		struct propSet *set2;

		set2 = get_result(set->nodes,"serviceSystem");
		if (set2) {
			ss = malloc(sizeof(struct ManagedObjectReference *));
			ss->type = set2->type;
			ss->value= set2->value;
			return ss;
		} else {
			printf("getserviceSystem: unable to find serviceSystem in results!\n");
		}
	} else {
		printf("getserviceSystem: unable to find configManager in results!\n");
		return 0;
	}
	
	return 0;
}

int usage(int r) {
	printf("usage: esxshell [options] start|stop <esx hostname>\n");
	printf("   where options are:\n");
	printf("    -u <user>           username\n");
	printf("    -p <pass>           password\n");
	exit(r);
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char server[128], *user, *pass, *p;
	int port, ch, r, op;
#if 0
	struct ManagedObjectReference *mor;
	struct returnval *ret;
	struct propSet *set,*set2,*set3;
	int count;
	list res;
	char *paths[] = { "serviceInfo", 0 };
#endif
	struct ServiceRequest req;

	server[0] = 0;
	port = 0;
	user = pass = 0;
	r = 1;
	op = -1;
	while((ch = getopt(argc, argv, "s:u:p:hcext:")) != -1) {
		switch(ch) {
		case 's':
			break;
		case 'u':
			user = optarg;
			break;
		case 'p':
			pass = optarg;
			break;
		case 'h':
			usage(0);
			break;
		}
	}

	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	if (strcmp(argv[optind],"start") == 0) op = 1;
	else if (strcmp(argv[optind],"stop") == 0) op = 0;
	optind++;

	/* sanity check op */
	dprintf("op: %d\n", op);
	if (op == -1) {
		printf("error: an operation MUST be specified");
		usage(1);
	}

	/* get host */
	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	server[0] = 0;
	strncat(server,argv[optind],sizeof(server));
	p = strchr(server,':');
	if (p) {
		strncat(server, server, p - server);
		dprintf("port: %s\n", p);
		port = atoi(p+1);
	} else {
		port = 0;
	}

	dprintf("server: %s, port: %d\n", server, port);
	vim = vim_connect(server, port);
	if (!vim) {
//		printf("vim_connect failed for server: %s\n", server);
		r = VIM_CONNECT;
		goto done;
	}

        if (!user && strcmp(vim->sc->about->apiType, "VirtualCenter") == 0) {
		char vcuser[64];

                if (!get_vcuser(server,port,vcuser))
                        user = vcuser;
        }
        if (!user) user = "root";

	if (vim_login(vim, user, pass)) {
//		dprintf("error logging in\n");
		r = VIM_LOGIN;
		goto done;
	}

	/* Setup the request and send it */
	req.serviceManager = getserviceSystem(vim);
	if (!req.serviceManager) {
		r = VIM_NOT_FOUND;
		goto done;
	}

	req.id = "TSM-SSH";
	if (op == 0) {
		dprintf("stopping...\n");
		StopService(vim->soap,vim->endpoint,&req);
	} else {
		dprintf("starting...\n");
		StartService(vim->soap,vim->endpoint,&req);
	}

	req.id = "TSM";
	if (op == 0) {
		dprintf("stopping...\n");
		StopService(vim->soap,vim->endpoint,&req);
	} else {
		dprintf("starting...\n");
		StartService(vim->soap,vim->endpoint,&req);
	}

	r = 0;

#if 0
	mor = getserviceSystem(vim);
	printf("mor: %p\n", mor);
	if (!mor) {
		r = VIM_NOT_FOUND;
		goto done;
	}
	dumpmor(mor);

	res = vim_getinfo(vim,mor->type,paths,0,mor);
	count = list_count(res);
	dprintf("count: %d\n", count);
	if (count < 1) {
		printf("esxshell: unable to get services!\n");
		return 0;
	}
	ret = res->first->item;
	dumpmor(&ret->obj);
	list_reset(ret->nodes);
	while((set = list_get_next(ret->nodes)) != 0) {
		printf("name: %s\n", set->name);
		list_reset(set->nodes);
		while((set2 = list_get_next(set->nodes)) != 0) {
			printf("name: %s\n", set2->name);
			p = get_result_value(set2->nodes,"key");
			printf("p: %s\n", p);
			if (strcmp(p,"TSM-SSH") == 0) {
				p = get_result_value(set2->nodes,"running");
			}
		}
	}
#endif

	if (vim) vim_disconnect(vim);
done:
	dprintf("r: %d\n", r);
	return r;
}
