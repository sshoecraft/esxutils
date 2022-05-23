
#include "esxpost.h"
#include "vim.h"
#include "soapfuncs/soapfuncs.h"
#include "soapfuncs/ServiceContent.h"
#include "soapfuncs/Destroy.h"

#define HOST_USER "root"

/* Add a host */
struct ManagedObjectReference *add_host(struct esxpost *info, struct ManagedObjectReference *cl_obj) {
	struct ManagedObjectReference *host_obj;
	char pass[128], *license, *tp, *p;

	tp = (strlen(info->key) ? info->key : 0);
	license = (strlen(info->license) ? info->license : 0);
	if (license) dprintf("selected license: %s\n", license);
	p = vim_cred_get(info->fqdn,HOST_USER);
	if (!p) {
		printf("error: unable to get credentials for %s user on %s!\n", HOST_USER, info->fqdn);
		return 0;
	}
	pass[0] = 0;
	strncat(pass,p,sizeof(pass)-1);
	host_obj = vim_add_host(info->vim, cl_obj, info->fqdn, 0, HOST_USER, pass, license, tp);
	if (!host_obj) {
		printf("error: unable to add host to VC!\n");
		return 0;
	}

	return host_obj;
}

/* Reconnect a host */
int con_host(struct esxpost *info, struct ManagedObjectReference *host_obj) {
	char pass[128], *tp, *p;

	/* sslThumbprint */
	tp = (strlen(info->key) ? info->key : 0);

	dprintf("reconnecting...\n");
	p = vim_cred_get(info->fqdn,HOST_USER);
	if (!p) {
		printf("error: unable to get credentials for %s user on %s!\n", HOST_USER, info->fqdn);
		return 1;
	}
	pass[0] = 0;
	strncat(pass,p,sizeof(pass)-1);
	/* First, reconnect with no opts */
//	if (vim_reconnect_host(info->vim, host_obj, 0, 0, 0, 0, 0)) {
		if (vim_reconnect_host(info->vim, host_obj, info->fqdn, 0, HOST_USER, pass, tp)) {
			return 1;
		}
//	}

	return 0;
}
