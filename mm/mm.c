
#include <unistd.h>
#include "vim.h"
#ifndef _WIN32
#include "dns.c"
#include "db.h"
#endif

#define GENERIC_ERROR	1
#define DNS_NOT_FOUND	2
#define DB_CONNECT	3
#define DB_NOT_FOUND	4
#define VIM_CONNECT	5
#define VIM_LOGIN	6
#define VIM_NOT_FOUND	7

#define DB_NAME "esxadmin"
#define DB_USER "esxadmin_ro"
#define DB_PASS 0

int usage(int r) {
	printf("usage: mm [options] <esx hostname>\n");
	printf("   where options are:\n");
	printf("    -s <host>[:port]    vc server\n");
	printf("    -u <user>           username\n");
	printf("    -p <pass>           password\n");
	printf("    -c                  check if in maint mode\n");
	printf("    -e                  enter maint mode\n");
	printf("    -x                  exit maint mode\n");
	printf("    -t <seconds>        timeout for enter/exit\n");
	exit(r);
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char server[128], host[128], *user, *pass, *p;
	int port, ch, r;
	struct ManagedObjectReference *host_obj;
	int check,enter,doexit,timeout;
#ifndef __WIN32
	struct hostent *he;
#endif

	server[0] = 0;
	port = 0;
	user = pass = 0;
	check = enter = doexit = timeout = 0;
	r = GENERIC_ERROR;
	while((ch = getopt(argc, argv, "s:u:p:hcext:")) != -1) {
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
		case 'c':
			check = 1;
			break;
		case 'e':
			enter = 1;
			break;
		case 'x':
			doexit = 1;
			break;
		case 'h':
			usage(0);
			break;
		case 't':
			timeout = atoi(optarg);
			break;
		}
	}
	/* sanity check opts */
	if (check == 0 && enter == 0 && doexit == 0) {
		printf("error: an operation MUST be specified");
		usage(1);
	}

	/* get host */
	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	host[0] = 0;
#ifndef __WIN32
	he = getdnshostent(argv[optind]);
	if (!he) {
//		printf("error: host not found in dns.\n");
		r = DNS_NOT_FOUND;
		goto done;
	}
	strncat(host,he->h_name,sizeof(host)-1);
#else
	strncat(host,argv[optind],sizeof(host)-1);
#endif
	dprintf("host: %s\n", host);

	if (!strlen(server)) {
#ifdef __WIN32
		usage(1);
#else
		char query[128];
		DB db;
		int found;

		if (db_connect(&db,DB_NAME,DB_USER,DB_PASS)) {
//			printf("cant connect to db to get vc server name, please specify.\n");
			r = DB_CONNECT;
			goto done;
		}
		found = 0;
		sprintf(query,"select server from farms where id = (select farm_id from hosts where name = '%s');\n",host);
		if (db_exec(&db,query) == 0) {
			if (db_fetch(&db) == 0) {
				SQLGetData(db.hstmt, 1, SQL_C_CHAR, (SQLPOINTER) server, sizeof(server), 0);
				dprintf("server: %s\n", server);
				found = 1;
			}
			db_fetch_done(&db);
		}
		if (!found) {
//			printf("cant find that host in db, please specify server name\n");
			r = DB_NOT_FOUND;
			goto done;
		}
		db_disconnect(&db);
#endif
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

	host_obj = vim_findobj(vim, "HostSystem", host);
	if (!host_obj) {
//		printf("unable to find host: %s on: %s\n", host, server);
		r = VIM_NOT_FOUND;
		goto done;
	}

	if (check) {
		r = (vim_ismm(vim, host_obj) == 0);
	} else if (enter) {
		r = vim_entermm(vim, host_obj, timeout);
	} else if (doexit) {
		r = vim_exitmm(vim, host_obj, timeout);
	}

	if (vim) vim_disconnect(vim);
done:
	dprintf("r: %d\n", r);
	if (check) printf("%s", (r ? "false\n" : "true\n") );
#ifdef _WIN32
	exit(r);
#else
	return r;
#endif
}
