
#include "HostConfigSpec.h"

int put_HostConfigSpec(struct soap *soap, char *tag, void **ptr, int req) {
	struct HostConfigSpec *spec = *ptr;
	if (spec) {
		struct mydesc put_desc[] = {
#if 0
#if 0
			{ "nasDatastore", &spec->nasDatastore, get_nasDatastore;, 0 },
			{ "network", &spec->network, get_HostNetworkConfig, 0 },
			{ "nicTypeSelection", &spec->nicTypeSelection, get_nicTypeSelection;, 0 },
#endif
			{ "service", &spec->service, get_service;, 0 },
#if 0
			{ "firewall", &spec->firewall, get_HostFirewallConfig, 0 },
			{ "option", &spec->option, get_option;, 0 },
			{ "datastorePrincipal", &spec->datastorePrincipal, get_char, 0 },
			{ "datastorePrincipalPasswd", &spec->datastorePrincipalPasswd, get_char, 0 },
			{ "datetime", &spec->datetime, get_HostDateTimeConfig, 0 },
			{ "storageDevice", &spec->storageDevice, get_HostStorageDeviceInfo, 0 },
			{ "license", &spec->license, get_HostLicenseSpec, 0 },
			{ "security", &spec->security, get_HostSecuritySpec, 0 },
			{ "userAccount", &spec->userAccount, get_userAccount;, 0 },
			{ "usergroupAccount", &spec->usergroupAccount, get_usergroupAccount;, 0 },
			{ "memory", &spec->memory, get_HostMemorySpec, 0 },
#endif
#endif
			{ 0,0,0,0 }
		};


//		dprintf("HostConfigSpec: hostName: %s, port: %d, sslThumbprint: %s, userName: %s, password: %s, force: %d\n", spec->hostName, spec->port, spec->sslThumbprint, spec->userName, spec->password, spec->force);
		return soap_send_desc(soap, tag, "xsi:type", "HostConfigSpec", put_desc);
	}
	return 0;
}
