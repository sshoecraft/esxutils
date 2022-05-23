
#include <unistd.h>
#include "vim.h"
#include "soapfuncs/ReconfigureCluster.h"
#include "dns.c"
#include "db.h"

#define DB_NAME "esxadmin"
#define DB_USER "esx_ro"
#define DB_PASS 0

int usage(int r) {
	printf("usage: ac [options] <farm>\n");
	printf("   where options are:\n");
	printf("    -s <server>         virtual center server\n");
	printf("    -u <user>           username\n");
	printf("    -p <pass>           password\n");
	printf("    -e                  enable admin control\n");
	printf("    -d                  disable admin control\n");
	printf("    -t <seconds>        timeout for enable/disable\n");
	exit(r);
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char server[64], *user, *pass, *name, *p;
	int port, ch, r;
	struct ManagedObjectReference *cluster_obj,*task;
	struct ReconfigureClusterRequest req;
	struct ClusterDasConfigInfo das;
	struct ClusterDrsConfigInfo drs;
	struct ClusterConfigSpec spec;
	int enable,disable,timeout;

	server[0] = 0;
	port = 0;
	user = pass = 0;
	enable = disable = timeout = 0;
	while((ch = getopt(argc, argv, "s:u:p:hted")) != -1) {
		switch(ch) {
		case 's':
			server[0] = 0;
			p = strchr(optarg,':');
			if (p) {
				strncat(server, optarg, p - optarg);
				dprintf("port: %s\n", p);
				port = atoi(p+1);
			} else {
				strncat(server, optarg, sizeof(server)-1);
				port = 0;
			}
			break;
		case 'u':
			user = optarg;
			break;
		case 'p':
			pass = optarg;
			break;
		case 'e':
			enable = 1;
			break;
		case 'd':
			disable = 1;
			break;
		case 'h':
			usage(0);
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		}
	}
	if (enable == 0 && disable == 0) {
		printf("error: an operation MUST be specified");
		usage(1);
	}

	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	name = argv[optind];
	dprintf("name: %s\n", name);

	if (!strlen(server)) {
		char query[128];
		DB db;
		int found;

		if (db_connect(&db,DB_NAME,DB_USER,DB_PASS)) {
			printf("cant connect to db to get vc server name, please specify.\n");
			return 1;
		}
		found = 0;
		sprintf(query,"select server from farms where name = '%s'",name);
		if (db_exec(&db,query) == 0) {
			if (db_fetch(&db) == 0) {
				SQLGetData(db.hstmt, 1, SQL_C_CHAR, (SQLPOINTER) server, sizeof(server), 0);
				dprintf("server: %s\n", server);
				found = 1;
			}
			db_fetch_done(&db);
		}
		if (!found) {
			printf("cant find that farm in db, please specify server name\n");
			return 1;
		}
		db_disconnect(&db);
	}

	dprintf("server: %s, port: %d\n", server, port);
	vim = vim_connect(server, port);
	if (!vim) {
		printf("vim_connect failed for server: %s\n", server);
		return 1;
	}

        if (!user && strcmp(vim->sc->about->apiType, "VirtualCenter") == 0) {
                char vcuser[64];

                if (!get_vcuser(server,port,vcuser))
                        user = vcuser;
        }
        if (!user) user = "root";

	dprintf("user: %s, pass: %s\n", user, pass);
	if (vim_login(vim, user, pass)) {
		dprintf("error logging in\n");
		return 1;
	}

	cluster_obj = vim_findobj(vim, "ClusterComputeResource", name);
	if (!cluster_obj) {
		printf("unable to find farm: %s on: %s\n", name, server);
		return 1;
	}

	/* Get the cluster config */
	memset(&das,0,sizeof(das));
//	das.failoverLevel = "1";
	das.admissionControlEnabled = (enable ? 1 : -1);
//	das.enabled = 1;
//	das.option = list_create();
//	add_das_options(&das);

	memset(&drs,0,sizeof(drs));
	drs.enabled = 1;

	memset(&spec,0,sizeof(spec));
	spec.dasConfig = &das;
//	spec.drsConfig = &drs;

	req.entity = cluster_obj;
	req.spec = &spec;
//	req.modify = -1;
	req.modify = 1;

	r = 0;
	r = ReconfigureCluster(vim->soap, vim->endpoint, &req, &task);
	if (!r) r = vim_wait4task(vim,task,timeout);

	if (vim) vim_disconnect(vim);
	dprintf("returning: %d\n", r);
	return r;
}
