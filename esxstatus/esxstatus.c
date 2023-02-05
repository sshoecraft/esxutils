
#ifndef __WIN32
#include <unistd.h>
#else
#include <windows.h>
#endif
#include "vim.h"
#include "soapfuncs/RetrieveProperties.h"
#ifndef __WIN32
#include <signal.h>
#endif

char server[64];
int state;
#ifndef __WIN32
static void _do_exit(int sig) {
	switch(state) {
	case 1:
		printf("%s: timeout connecting to server\n", server);
		break;
	case 2:
		printf("%s: timeout logging in to server\n", server);
		break;
	case 3:
		printf("%s: timeout getting status from\n", server);
		break;
	}
	exit(1);
}
#endif

int usage(int r) {
	printf("usage: esxstatus [options] <hostname>\n");
	printf("   where options are:\n");
	printf("    -b                  get boot time\n");
	printf("    -u <user>           username\n");
	printf("    -p <pass>           password\n");
#ifndef __WIN32
	printf("    -t <seconds>        timeout\n");
#endif
	exit(r);
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char *user, *pass, *p, status[128], boot[32];
	int port, ch, r, timeout, getboot;
	int gotstat, gotboot;
	list results;
	char *paths[] = { "overallStatus", "summary.runtime.bootTime", 0 };
	struct returnval *ret;

	server[0] = 0;
	port = timeout = getboot = 0;
	user = pass = 0;
	while((ch = getopt(argc, argv, "bu:p:t:")) != -1) {
		switch(ch) {
		case 'b':
			getboot = 1;
			break;
		case 'u':
			user = optarg;
			break;
		case 'p':
			pass = optarg;
			break;
#ifndef __WIN32
		case 't':
			timeout = atoi(optarg);
			break;
#endif
		case '?':
			usage(0);
			break;
		default:
			usage(1);
			break;
		}
	}

	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	optarg = argv[optind];
	dprintf("optarg: %s\n", optarg);
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

	dprintf("user: %p\n",user);
	if (!user) {
		user = USER;
		pass = PASS;
	}

	state = 0;
#ifndef __WIN32
	signal(SIGALRM, _do_exit);
#endif

	dprintf("server: %s, port: %d\n", server, port);
	state = 1;
#ifndef __WIN32
	alarm(15);
#endif
	r = 1;
	vim = vim_connect(server, port);
	if (!vim) {
		printf("%s: error connecting to server\n", server);
		goto done;
	}

	dprintf("user: %s, pass: %s\n", user, (pass ? pass : "(null)"));
	state = 2;
#ifndef __WIN32
	alarm(60);
#endif
	if (vim_login(vim, user, pass)) {
		printf("%s: error logging in to server\n", server);
		goto done;
	}

	results = vim_getinfo(vim, "HostSystem", paths, 0, 0);
	state = 3;
#ifndef __WIN32
	alarm(30);
#endif
	if (list_count(results) < 1) {
		printf("%s: unable to get status from server\n", server);
		goto done;
	}

	gotstat = gotboot = 0;
        list_reset(results);
        while((ret = list_get_next(results)) != 0) {
		p = get_result_value(ret->nodes,"overallStatus");
		if (p) {
			if (strcmp(p,"green") == 0) strcpy(status,"OK");
			else strcpy(status,p);
			gotstat = 1;
		}
		p = get_result_value(ret->nodes,"summary.runtime.bootTime");
		if (p) {
			strcpy(boot,p);
			gotboot = 1;
		}
#if 0
			else if (strcmp(p,"gray") == 0) printf("unknown");
			else {
				set = get_result(ret->nodes, "declaredAlarmState");
				if (!set) return 0;
				dprintf("set->name: %s\n", set->name);
				list_reset(set->nodes);
				while((set2 = list_get_next(set->nodes)) != 0) {
					dprintf("set2->name: %s\n", set2->name);
					p = get_result_value(set2->nodes, "overallStatus");
					dprintf("p: %s\n", p);
					if (!p) continue;
					if (strcmp(p,"green") == 0 || strcmp(p,"gray") == 0) continue;
					set3 = get_result(set2->nodes, "alarm");
					if (!set3) {
						dprintf("set3 is null!\n");
						continue;
					}
					sprintf(object,"%s:%s", set3->type, set3->value);
					alarm = get_alarm(s, object);
					if (!alarm) continue;
					dprintf("adding alarm: %s\n", alarm->name);
					list_add(host->alarms,alarm->name,strlen(alarm->name)+1);
				}
			}
		}
#endif
        }
	if (!gotstat) printf("%s: unable to get status from server\n", server);
	if (getboot) {
		if (gotboot) {
			printf("%s: %s %s\n", server, status, boot);
			r = 0;
		} else {
			printf("%s: unable to get bootTime from server\n", server);
		}
	} else {
		printf("%s: %s\n", server, status);
		r = 0;
	}

done:
	if (vim) {
		state = 4;
#ifndef __WIN32
		alarm(10);
#endif
		vim_disconnect(vim);
	}

	dprintf("returning: %d\n", r);
	return r;
}
