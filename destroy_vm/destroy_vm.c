
#include <vim.h>
#include "db.h"
#include "gendb_vms.h"
#include "gendb_farms.h"

#include "soapfuncs/ManagedObjectReference.h"
#include "soapfuncs/RetrieveProperties.h"
#include "soapfuncs/Service.h"

#define VIM_CONNECT     5
#define VIM_LOGIN       6
#define VIM_NOT_FOUND   7

#define DB_NAME "esxadmin"
#define DB_USER "esxadmin_ro"
#define DB_PASS "esxadmin_ro"

#define dumpmor(obj) printf("type: %s, val: %s\n", (obj)->type, (obj)->value);

int usage(int r) {
	printf("usage: destroy_vm [-s server] [-u user] [-p password] vm-name [vm name...]\n");
	printf("  where options are:\n");
	printf("    -s             server to connect to\n");
	printf("    -u             username\n");
	printf("    -p             password\n");
	exit(r);
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char server[128], *user, *pass, *name, *p;
	int port, r, ch;
	struct ManagedObjectReference *mor;
	DB dbc, *db;

	user = pass = 0;
	r = 1;
	*server = 0;
//	op = -1;
//	dprintf("argc: %d\n", argc);
//	if (argc < 5) usage(1);

//	for(r=0; r < argc; r++) dprintf("arg[%d]: %s\n", r, argv[r]);

	while((ch = getopt(argc, argv, "acs:u:p:")) != -1) {
		switch(ch) {
		case 's':
			strncpy(server, optarg, sizeof(server)-1);
			break;
		case 'u':
			user = optarg;
			break;
		case 'p':
			pass = optarg;
			break;
		case '?':
			usage(0);
		}
	}

	db = 0;
	if (db_connect(&dbc,DB_NAME,DB_USER,DB_PASS) == 0) db = &dbc;

	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	name = argv[optind++];
	if (!strlen(server)) {
		struct vms_record vm;
		char clause[128];

		if (!db) {
			printf("error: unable to find server for %s, please specify\n", name);
			return 1;
		}
		sprintf(clause,"WHERE name = '%s'", name);
		if (vms_select_record(db,&vm,clause) == 0) {
			struct farms_record farm;

			sprintf(clause,"WHERE id = '%d'", vm.farm_id);
			if (farms_select_record(db,&farm,clause)) {
				printf("error: unable to find server for %s, please specify\n", name);
				return 1;
			}
			strncpy(server,farm.server,sizeof(server)-1);
		}
	}

	p = strchr(server,':');
	if (p) {
//		strncat(server, server, p - server);
		*p = 0;
		dprintf("port: %s\n", p+1);
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

	dprintf("about->type: %s\n", vim->sc->about->apiType);
        if (!user && strcmp(vim->sc->about->apiType, "VirtualCenter") == 0) {
                char vcuser[64];

                if (!get_vcuser(server,port,vcuser))
                        user = vcuser;
        }
	if (!user) {
		printf("unable to determine user.\n");
		return 1;
	}
	pass = 0;

	dprintf("logging in...\n");
        if (vim_login(vim, user, pass)) {
//              dprintf("error logging in\n");
                r = VIM_LOGIN;
                goto done;
        }

	dprintf("getting MoR\n");
	mor = vim_findobj(vim, "VirtualMachine", name);
	dprintf("mor: %p\n", mor);
	if (!mor) {
		printf("unable to find vm: %s\n", name);
		return 1;
	}
	dprintf("powering off...\n");
	vim_poweroff(vim,mor);
	dprintf("destroying...\n");
	r = vim_destroy(vim,mor);

	if (vim) vim_disconnect(vim);
done:
	dprintf("r: %d\n", r);
	return r;
}
