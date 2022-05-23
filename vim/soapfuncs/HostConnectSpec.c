
#include "HostConnectSpec.h"

int put_HostConnectSpec(struct soap *soap, char *tag, void **ptr, int req) {
	struct HostConnectSpec *spec = *ptr;
	if (spec) {
		struct mydesc put_hcs_desc[] = {
			{ "hostName", &spec->hostName, put_char, 0 },
			{ "port", &spec->port, put_int, 0 },
			{ "sslThumbprint", &spec->sslThumbprint, put_char, 0 },
			{ "userName", &spec->userName, put_char, 0 },
			{ "password", &spec->password, put_char, 0 },
			{ "force", &spec->force, put_bool, 1 },			/* req'd */
//			{ "vimAccountName", &spec->vimAccountName, put_char, 0 },
//			{ "vimAccountPassword", &spec->vimAccountPassword, put_char, 0 },
//			{ "vmFolder", &spec->vmFolder, put_ManagedObjectReference, 0 },
			{ 0,0,0,0 }
		};


		dprintf("HostConnectSpec: hostName: %s, port: %d, sslThumbprint: %s, userName: %s, password: %s, force: %d\n", spec->hostName, spec->port, spec->sslThumbprint, spec->userName, spec->password, spec->force);
		return soap_send_desc(soap, tag, "xsi:type", "HostConnectSpec", put_hcs_desc);
	}
	return 0;
}

