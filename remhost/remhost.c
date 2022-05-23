
#include "vim.h"

int main(int argc, char **argv) {
	struct vim_session *vim;
        struct ManagedObjectReference *host_obj;
	char vcserver[64],vcuser[32],name[64];
	int vcport = 0;

	if (argc < 3) {
		printf("usage: remhost <VC server> <hostname>\n");
		return 1;
	}
	strcpy(vcserver,argv[1]);
	strcpy(name,argv[2]);

        dprintf("connecting...\n");
        if ((vim = vim_connect(vcserver,vcport)) == 0) {
                printf("error: unable to connect to vcserver: %s\n", vcserver);
		return 1;
        }
        dprintf("logging in...\n");
	if (get_vcuser(vcserver,vcport,vcuser)) {
		printf("error: unable to get vcuser for server: %s\n", vcserver);
		return 1;
	}
        if (vim_login(vim,vcuser,0)) {
                printf("error: unable to login to vcserver: %s\n", vcserver);
		return 1;
        }

        /* Find the host */
        dprintf("finding host...\n");
        host_obj = vim_findobj(vim, "HostSystem", name);
        if (host_obj) {
                /* If the host is disconnected */
                dprintf("checking conx status...\n");
                if (vim_isconnected(vim, host_obj)) {
			if (vim_disconnect_host(vim, host_obj, 0)) {
				printf("error: unable to disconnect host!\n");
				return 1;
			}
                }
		if (vim_destroy(vim,host_obj)) {
			printf("error unable to remove host!\n");
			return 1;
                }
        }

	return 0;
}
