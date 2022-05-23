
#ifndef __VIM_HostConfigSpec_H
#define __VIM_HostConfigSpec_H

#include "soapfuncs.h"
#include "list.h"

struct HostConfigSpec {
//        struct HostNasVolumeConfig *nasDatastore;  /* optional element of type ns1:HostNasVolumeConfig */
	list nasDatastore;
        struct HostNetworkConfig *network; /* optional element of type ns1:HostNetworkConfig */
//        struct HostVirtualNicManagerNicTypeSelection *nicTypeSelection;    /* optional element of type ns1:HostVirtualNicManagerNicTypeSelection */
	list nicTypeSelection;
//        struct HostServiceConfig *service; /* optional element of type ns1:HostServiceConfig */
	list service;
        struct HostFirewallConfig *firewall;       /* optional element of type ns1:HostFirewallConfig */
//        struct OptionValue *option;        /* optional element of type ns1:OptionValue */
	list option;
        char *datastorePrincipal;       /* optional element of type xsd:string */
        char *datastorePrincipalPasswd; /* optional element of type xsd:string */
        struct HostDateTimeConfig *datetime;       /* optional element of type ns1:HostDateTimeConfig */
        struct HostStorageDeviceInfo *storageDevice;       /* optional element of type ns1:HostStorageDeviceInfo */
        struct HostLicenseSpec *license;   /* optional element of type ns1:HostLicenseSpec */
        struct HostSecuritySpec *security; /* optional element of type ns1:HostSecuritySpec */
//        struct HostAccountSpec *userAccount;       /* optional element of type ns1:HostAccountSpec */
	list userAccount;
//        struct HostAccountSpec *usergroupAccount;  /* optional element of type ns1:HostAccountSpec */
	list usergroupAccount;
        struct HostMemorySpec *memory;     /* optional element of type ns1:HostMemorySpec */
};

int get_HostConnectInfo(struct soap *soap, char *tag, void **ptr, int req);

#endif
