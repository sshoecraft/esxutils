
#include <vim.h>

#include "soapfuncs/ManagedObjectReference.h"
#include "soapfuncs/RetrieveProperties.h"
#include "soapfuncs/Service.h"

#define VIM_CONNECT     5
#define VIM_LOGIN       6
#define VIM_NOT_FOUND   7

#define dumpmor(obj) printf("type: %s, val: %s\n", (obj)->type, (obj)->value);

int usage(int r) {
	printf("usage: promote vsphere-server vsphere-user vsphere-pass vm-name\n");
	exit(r);
}

int do_destroy(struct vim_session *vim, char *name) {
	struct ManagedObjectReference *mor;

	mor = vim_findobj(vim, "VirtualMachine", name);
	dprintf("mor: %p\n", mor);
	if (!mor) {
//		printf("unable to find vm: %s\n", name);
		return 1;
	}

	return vim_destroy(vim,mor);
}

int do_rename(struct vim_session *vim, char *from, char *to) {
	struct ManagedObjectReference *mor;
	int r;

	dprintf("from: %s, to: %s\n", from, to);
	mor = vim_findobj(vim, "VirtualMachine", from);
	dprintf("mor: %p\n", mor);
	if (!mor) {
//		printf("unable to find vm: %s\n", from);
		return 1;
	}
	r = vim_rename(vim, mor, to);
	dprintf("r: %d\n", r);
	return r;
}

int main(int argc, char **argv) {
	struct vim_session *vim;
	char server[128], *user, *pass, *p;
	char name[256],prev_name[256],new_name[256],old_name[256];
	int port, r;

	server[0] = 0;
	port = 0;
	user = pass = 0;
	r = 1;
//	op = -1;
	dprintf("argc: %d\n", argc);
	if (argc < 5) usage(1);

	for(r=0; r < argc; r++) dprintf("arg[%d]: %s\n", r, argv[r]);

	server[0] = 0;
	strncat(server,argv[1],sizeof(server)-1);
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

	user = argv[2];
	if (!user) {
		r = VIM_LOGIN;
		goto done;
	}
	pass = argv[3];
	if (!pass) {
		r = VIM_LOGIN;
		goto done;
	}

	if (vim_login(vim, user, pass)) {
//		dprintf("error logging in\n");
		r = VIM_LOGIN;
		goto done;
	}

	snprintf(name,sizeof(name)-1,"%s",argv[4]);
	snprintf(old_name,sizeof(old_name)-1,"%s-old",argv[4]);
	snprintf(prev_name,sizeof(prev_name)-1,"%s-prev",argv[4]);
	snprintf(new_name,sizeof(new_name)-1,"%s-new",argv[4]);

	/* If there's no -new, dont bother */
	{
		struct ManagedObjectReference *mor;

		mor = vim_findobj(vim, "VirtualMachine", new_name);
//		printf("-new mor: %p\n", mor);
		if (!mor) return 0;
	}

	/* Rename -prev to -old */
	do_rename(vim, prev_name, old_name);

	/* Rename the current to -prev */
	do_rename(vim, name, prev_name);

	/* Rename the -new to current */
	if (do_rename(vim, new_name, name)) {
		/* Rename failed, change it back */
		do_rename(vim, prev_name, name);
		do_rename(vim, old_name, prev_name);
	} else {
		/* Destroy the old vm */
		do_destroy(vim, old_name);
	}

	r = 0;

	if (vim) vim_disconnect(vim);
done:
	dprintf("r: %d\n", r);
	return r;
}
