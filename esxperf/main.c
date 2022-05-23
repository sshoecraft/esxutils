
#include "vim.h"
#include "db.h"
#include "version.h"
#include "soapfuncs/RetrieveProperties.h"
#include "soapfuncs/RetrieveServiceContent.h"

#define VM_METRICS 0

#if DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(format, args...) /* noop */
#endif

int verbose = 0;
int debug = 0;

void usage(void) {
	printf("usage: esxconf [-cn] [-s <server>[:<port>]] [-u <username> -p <password>] [-o <obj>] [-t <type>] [<path> ... <path>]\n");
	printf("  where:\n");
	printf("    -s             server/port to connect to\n");
	printf("    -u             username\n");
	printf("    -p             password\n");
	printf("    -h             this listing\n");
	printf("    -v             be verbose\n");
	printf("    -V             display version\n");
	exit(1);
}

extern list getperf(struct vim_session *, char *);

#define DB_NAME "esxadmin"
#define DB_USER "esxadmin"
#define DB_PASS "G3tB3nt"

int get_host_metrics(struct vim_session *s, DB *db, char *name, struct ManagedObjectReference *mo_ref, int *);
int get_vm_metrics(struct vim_session *s, DB *db, char *name, struct ManagedObjectReference *mo_ref, int);

int main(int argc, char **argv) {
	char *dbstr, server[64], *user, *pass, *p;
	struct vim_session *s;
	int r, port, ch;
	list hosts,vms;
	int host_id;
	DB db;
	char *host_paths[] = { "name", "vm", 0 }, *vm_paths[] = { "name", "summary.runtime.powerState", 0 };
	struct returnval *ret;
	struct propSet *set,*set2;
	struct ManagedObjectReference *mor;
	char *host_name,*vm_name,*powerState;
	int host_count,vm_count;

	server[0] = 0;
	dbstr = user = pass = 0;
	debug = port = 0;
	while((ch = getopt(argc, argv, "s:u:p:hv")) != -1) {
		switch(ch) {
		case 'd':
			debug++;
			break;
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
		case 'h':
			usage();
			break;
		case 'v':
			verbose = 1;
			break;
		case 'V':
			printf("esxpost version %s\n", VERSIONSTR);
			printf("by Steve Shoecratft (sshoecraft@hp.com)\n");
			return 0;
                        break;
		}
	}
	dprintf("*server: %d, user: %p\n", *server, user);
	if (!server[0]) strcpy(server, "localhost");
	dprintf("server: %s, port: %d, user: %s, pass: %s\n", server, port, user, pass);

	r = 1;

	/* Connect to server */
	dprintf("Connecting...\n");
	s = vim_connect(server, port);
	if (!s) {
		printf("error connecting to: %s\n", server);
		return 1;
	}

        if (!user && strcmp(s->sc->about->apiType, "VirtualCenter") == 0) {
                char vcuser[64];

                if (!get_vcuser(server,port,vcuser))
                        user = vcuser;
        }
        if (!user) user = "root";
        if (vim_login(s, user, pass)) goto done;

	dprintf("db_name: %s, db_user: %s, db_pass: %s\n", DB_NAME, DB_USER, DB_PASS);
	if (db_connect(&db,DB_NAME,DB_USER,DB_PASS)) goto done;

	hosts = vim_getinfo(s, "HostSystem", host_paths, 0, 0);
	dprintf("hosts count: %d\n", list_count(hosts));
	if (list_count(hosts) < 1) {
		printf("no hosts found: %s\n", server);
		goto done;
	}

	/* Process request */
	list_reset(hosts);
	while((ret = list_get_next(hosts)) != 0) {
		dprintf("----------------------------------------------------------------------\n");
		dprintf("Host Object: %s:%s\n", ret->obj.type, ret->obj.value);
		host_name = get_result_value(ret->nodes,"name");
		if (!host_name) {
			printf("unable to get name for host: %s:%s\n", ret->obj.type, ret->obj.value);
			continue;
		}
#if 0
		/* XXX sanity check name */
		if (strcmp(host_name,server) != 0 && strstr(host_name,server) == 0) {
			printf("VC: %s HOST: %s\n", server, host_name);
//			printf("WARNING: server is %s but host reports its name as %s!\n", server, host_name);
			continue;
		}
#endif

		dprintf("host_name: %s\n", host_name);
		host_count = get_host_metrics(s,&db,host_name,&ret->obj,&host_id);
		printf("%s: %d\n", host_name, host_count);
		if (host_count < 1) continue;

		vm_count = 0;
		set = get_result(ret->nodes,"vm");
		if (set) {
			list_reset(set->nodes);
			while((set2 = list_get_next(set->nodes)) != 0) {
				dprintf("----------------------------------------------------------------------\n");
//				dprintf("VM Object: %s:%s\n", set2->type, set2->value);
				mor = set2mor(set2);

				vms = vim_getinfo(s, "VirtualMachine", vm_paths, 0, mor);
				dprintf("vms count: %d\n", list_count(vms));
				if (list_count(vms) != 1) {
					/* XXX not an error */
					break;
				}

				list_reset(vms);
				while((ret = list_get_next(vms)) != 0) {
					dprintf("----------------------------------------------------------------------\n");
					dprintf("VM Object: %s:%s\n", ret->obj.type, ret->obj.value);

					vm_name = get_result_value(ret->nodes,"name");
					if (!vm_name) {
						printf("unable to get name for vm: %s:%s\n", ret->obj.type, ret->obj.value);
						continue;
					}
					dprintf("vm_name: %s\n", vm_name);
					powerState = get_result_value(ret->nodes,"summary.runtime.powerState");
					if (powerState && strcmp(powerState,"poweredOn") != 0) continue;
#if VM_METRICS
					vm_count = get_vm_metrics(s,&db,vm_name,&ret->obj,host_id);
					printf("  %s: %d\n", vm_name, vm_count);
#endif
				}
			}
		}
	}

	r = 0;

done:
	db_disconnect(&db);
	vim_disconnect(s);
	return r;
}
