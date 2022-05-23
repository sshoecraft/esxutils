
#include <stdio.h>
#include <termios.h>
#include "collect.h"

#ifdef __MINGW32__
#include "getopt.h"
#endif

/*
 * This program was written ~2008 to collect vmware info into a DB

1.1	?	init version
1.2	?	many changes
1.3 11/15/20	add vm_file collection
*/
char *version = "1.1";

void usage(void) {
	printf("usage: do_collect [-n] [-s <server>[:<port>]] [-u <username> -p <password>] [-a <actions file>\n");
	printf("  where:\n");
	printf("    -s             server/port to connect to\n");
	printf("    -u             username\n");
	printf("    -p             password\n");
	printf("    -h             this listing\n");
	printf("    -a             specify actions file\n");
	printf("    -n             don't update - just collect\n");
	exit(1);
}

char *actions_file;
int do_update,do_mksnap;

int main(int argc, char **argv) {
	char server[64], user[32], *pass, *p;
	int port, ch,prompt;
	char password[32];

	server[0] = 0;
	pass = actions_file = 0;
	do_update = 1;
	port = prompt = 0;
	user[0] = 0;
	while((ch = getopt(argc, argv, "a:s:u:p:hnmP")) != -1) {
		switch(ch) {
		case 'a':
			actions_file = optarg;
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
			strcpy(user,optarg);
			break;
		case 'p':
			pass = optarg;
			break;
		case 'h':
			usage();
			break;
		case 'm':
			do_mksnap = 1;
			break;
		case 'x':
			do_update = 0;
			break;
		case 'n':
			do_update = 0;
			break;
		case 'P':
			prompt = 1;
			break;
		}
	}
	if (!strlen(server)) usage();
	if (prompt) {
		if (vim_getpass(password,sizeof(password))) pass = password;
	}
	dprintf("*server: %d, port: %d, user: %p, pass: %p\n", *server, port, user, pass);

	if (server[0] == '@') {
		FILE *fp;

		strcpy(server,server+1);
		fp = fopen(server,"r");
		if (!fp) {
			perror("fopen");
			return 1;
		}
		while(fgets(server,sizeof(server),fp)) {
			server[strlen(server)-1] = 0;
			if (collect(server, port, user, pass))
				return 1;
		}
	} else {
		if (collect(server, 0, user, pass))
			return 1;
	}

	return 0;
}
