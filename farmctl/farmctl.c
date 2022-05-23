
#include <unistd.h>
#include "vim.h"
#include "soapfuncs/ReconfigureCluster.h"
#include "soapfuncs/RetrieveProperties.h"
#include "dns.c"
#include "db.h"

#define DB_NAME "esxadmin"
#define DB_USER "esxadmin_ro"
#define DB_PASS 0

int usage(int r) {
	printf("usage: ac [options] <farm>\n");
	printf("   where options are:\n");
	printf("    -s <server>         virtual center server\n");
	printf("    -u <user>           username\n");
	printf("    -p <pass>           password\n");
	printf("    -t <seconds>        timeout\n");
	printf("    -A                  enable admin control\n");
	printf("    -a                  disable admin control\n");
	printf("    -D                  enable DRS\n");
	printf("    -d                  disable DRS\n");
	printf("    -H                  enable HA\n");
	printf("    -h                  disable HA\n");
	printf("    -i                  set ilolation response\n");
	exit(r);
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char server[64], *user, *pass, *name, *p, *iresp;
	int port, ch, r;
	struct ManagedObjectReference *cluster_obj,*task;
	struct ReconfigureClusterRequest req;
	struct ClusterDasConfigInfo das;
	struct ClusterDasVmSettings dasVm;
	struct ClusterDrsConfigInfo drs;
	struct ClusterConfigSpec spec;
	int timeout,admin,drs_val,ha_val,iso_val,get;
	char iso[32];

	server[0] = 0;
	port = 0;
	user = pass = iresp = 0;
	admin = drs_val = ha_val = timeout = get = 0;
	while((ch = getopt(argc, argv, "s:u:p:tAaDdHhi:")) != -1) {
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
		case 'A':
			admin = 1;
			break;
		case 'a':
			admin = -1;
			break;
		case 'D':
			drs_val = 1;
			break;
		case 'd':
			drs_val = -1;
			break;
		case 'H':
			ha_val = 1;
			break;
		case 'h':
			ha_val = -1;
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		case 'i':
			if (strcmp(optarg,"none") == 0 || strcmp(optarg,"shutdown") == 0 || strcmp(optarg,"powerOff") == 0)
				iresp = optarg;
			else {
				printf("error: unknown isolation response: %s\n", optarg);
				usage(1);
			}
			break;
		case '?':
			usage(0);
			break;
		default:
			usage(1);
			break;
		}
	}
	/* if not op specified, default op is get */
	dprintf("admin: %d, drs: %d, ha: %d\n", admin, drs_val, ha_val);
	if (admin == 0 && drs_val == 0 && ha_val == 0 && iresp == 0) get = 1;
	/* allow setting of admin even if ha disabled */
//	if (admin == 1 && ha_val == 0) ha_val = 1;

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

	dprintf("user: %p\n",user);

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

	dprintf("user: %s, pass: %s\n", user, (pass ? pass : "(null)"));
	if (vim_login(vim, user, pass)) {
		dprintf("error logging in\n");
		return 1;
	}

	cluster_obj = vim_findobj(vim, "ClusterComputeResource", name);
	if (!cluster_obj) {
		printf("error: unable to find cluster '%s' on %s\n", name, server);
		return 1;
	}

	r = 0;
	if (get) {
		char **paths;
		struct vim_res2desc desc[] = {
			{ "configuration.dasConfig.enabled", VIM_TYPE_BOOL, &ha_val, 0, 0, -1 },
			{ "configuration.dasConfig.admissionControlEnabled", VIM_TYPE_BOOL, &admin, 0, 0, -1 },
			{ "configuration.dasConfig.defaultVmSettings.isolationResponse", VIM_TYPE_STRING, &iso, sizeof(iso), 0, -1 },
			{ "configuration.drsConfig.enabled", VIM_TYPE_BOOL, &drs_val, 0, 0, -1 },
			{ 0, 0, 0, 0, 0, 0 }
		};
		list results;
		struct returnval *ret;

                paths = build_paths(desc);
		if (paths) {
			results = vim_getinfo(vim, cluster_obj->type, paths, 0, cluster_obj);
			dprintf("count: %d\n", list_count(results));
			if (list_count(results) != 1) {
				printf("get_farm: count != 1 for: %s\n", cluster_obj->value);
				return 0;
			}
			ret = results->first->item;

			vim_results2desc(vim,ret->nodes,desc);

			if (strcmp(iso,"none") == 0)
				iso_val = 0;
			else if (strcmp(iso,"shutdown") == 0)
				iso_val = 1;
			else if (strcmp(iso,"powerOff") == 0)
				iso_val = 2;
			else {
				printf("error: unknown iso valuestr: %s\n", iso);
				iso_val = -1;
			}

			printf("CURRENT=HA:%d,Admin:%d,DRS:%d,ISO:%d\n",ha_val,admin,drs_val,iso_val);
		}
		dprintf("clust: %s:%s\n", cluster_obj->type, cluster_obj->value);
		exit(1);
	} else {
		memset(&das,0,sizeof(das));
//		das.failoverLevel = "1";
		das.admissionControlEnabled = admin;
		das.enabled = ha_val;

		if (iresp) {
			memset(&dasVm,0,sizeof(dasVm));
			dasVm.isolationResponse = iresp;
			das.defaultVmSettings = &dasVm;
		}

//		das.option = list_create();
//		add_das_options(&das);

		memset(&drs,0,sizeof(drs));
		drs.enabled = drs_val;

		memset(&spec,0,sizeof(spec));
		spec.dasConfig = &das;
		spec.drsConfig = &drs;

		req.entity = cluster_obj;
		req.spec = &spec;
//		req.modify = -1;
		req.modify = 1;

		r = ReconfigureCluster(vim->soap, vim->endpoint, &req, &task);
		if (!r) r = vim_wait4task(vim,task,timeout);
	}

	if (vim) vim_disconnect(vim);
	dprintf("returning: %d\n", r);
	return r;
}
