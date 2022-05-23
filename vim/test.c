
#include <stdio.h>
#include "vim.h"
#include "soapfuncs/RetrieveProperties.h"

#ifndef dprintf
#ifdef DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(format, args...) /* noop */
#endif
#endif

void _dnode(struct propSet *set) {
	if (set->type) printf("type: %s\n", set->type);
	if (set->value) printf("value: %s\n", set->value);
}

void _disp(list nodes, int level) {
        struct propSet *set;
        int first;

        first = 0;
        list_reset(nodes);
        while((set = list_get_next(nodes)) != 0) {
                if (!set->value) {
                        _disp(set->nodes,(first ? level+1 : level));
                } else {
                        _dnode(set);
                        first = 1;
                }
        }
}

int main(void) {
//	char *server = "g3t0001h.houston.hp.com";
	char *server = "westvc.houston.hp.com";
	int port = 0;
//	char *user = "sitescope";
	char *user = "americas\\shoecraf";
//	char *pass = "need2know";
	char *pass = 0;
	struct vim_session *s;
	struct ManagedObjectReference *mo_ref;
	char *host = "d9t0002h.houston.hp.com";
//	char *host = "d9t0068h.houston.hp.com";

        /* Connect to server */
        printf("Connecting...\n");
	s = vim_connect(server, port);
        if (!s) {
                printf("error connecting to: %s\n", server);
                return 1;
        }

        dprintf("Logging in...\n");
        if (vim_login(s, user, pass)) {
                printf("error logging into: %s\n", server);
		goto done;
        }

	mo_ref = vim_findobj(s, "HostSystem", host);
	dprintf("mo_ref: %p\n", mo_ref);
	if (!mo_ref) goto done;

//	vim_ismm(s, mo_ref);
	vim_reconnect_host(s, mo_ref, host, 0, "root", "urthe1", 0);

done:
	vim_disconnect(s);
	return 0;
}
