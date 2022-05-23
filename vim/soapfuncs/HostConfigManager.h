
#ifndef __VIM_HostConfigManager_H
#define __VIM_HostConfigManager_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct HostConfigManager {
	{ "cpuScheduler", &info->cpuScheduler, get_ManagedObjectReference, 0 },
	{ "datastoreSystem", &info->datastoreSystem, get_ManagedObjectReference, 0 },
	{ "memoryManager", &info->memoryManager, get_ManagedObjectReference, 0 },
	{ "storageSystem", &info->storageSystem, get_ManagedObjectReference, 0 },
	{ "networkSystem", &info->networkSystem, get_ManagedObjectReference, 0 },
	{ "vmotionSystem", &info->vmotionSystem, get_ManagedObjectReference, 0 },
	{ "virtualNicManager", &info->virtualNicManager, get_ManagedObjectReference, 0 },
	{ "serviceSystem", &info->serviceSystem, get_ManagedObjectReference, 0 },
	{ "firewallSystem", &info->firewallSystem, get_ManagedObjectReference, 0 },
	{ "advancedOption", &info->advancedOption, get_ManagedObjectReference, 0 },
	{ "diagnosticSystem", &info->diagnosticSystem, get_ManagedObjectReference, 0 },
	{ "autoStartManager", &info->autoStartManager, get_ManagedObjectReference, 0 },
	{ "snmpSystem", &info->snmpSystem, get_ManagedObjectReference, 0 },
	{ "dateTimeSystem", &info->dateTimeSystem, get_ManagedObjectReference, 0 },
	{ "patchManager", &info->patchManager, get_ManagedObjectReference, 0 },
	{ "imageConfigManager", &info->imageConfigManager, get_ManagedObjectReference, 0 },
	{ "bootDeviceSystem", &info->bootDeviceSystem, get_ManagedObjectReference, 0 },
	{ "firmwareSystem", &info->firmwareSystem, get_ManagedObjectReference, 0 },
	{ "healthStatusSystem", &info->healthStatusSystem, get_ManagedObjectReference, 0 },
	{ "pciPassthruSystem", &info->pciPassthruSystem, get_ManagedObjectReference, 0 },
	{ "licenseManager", &info->licenseManager, get_ManagedObjectReference, 0 },
	{ "kernelModuleSystem", &info->kernelModuleSystem, get_ManagedObjectReference, 0 },
	{ "authenticationManager", &info->authenticationManager, get_ManagedObjectReference, 0 },
	{ "powerSystem", &info->powerSystem, get_ManagedObjectReference, 0 },
	{ "cacheConfigurationManager", &info->cacheConfigurationManager, get_ManagedObjectReference, 0 },
	{ "esxAgentHostManager", &info->esxAgentHostManager, get_ManagedObjectReference, 0 },
	{ "iscsiManager", &info->iscsiManager, get_ManagedObjectReference, 0 },
	{ "vFlashManager", &info->vFlashManager, get_ManagedObjectReference, 0 },
	{ "vsanSystem", &info->vsanSystem, get_ManagedObjectReference, 0 },
	{ "messageBusProxy", &info->messageBusProxy, get_ManagedObjectReference, 0 },
	{ "userDirectory", &info->userDirectory, get_ManagedObjectReference, 0 },
	{ "accountManager", &info->accountManager, get_ManagedObjectReference, 0 },
	{ "hostAccessManager", &info->hostAccessManager, get_ManagedObjectReference, 0 },
	{ "graphicsManager", &info->graphicsManager, get_ManagedObjectReference, 0 },
	{ "vsanInternalSystem", &info->vsanInternalSystem, get_ManagedObjectReference, 0 },
	{ "certificateManager", &info->certificateManager, get_ManagedObjectReference, 0 },
	{ "cryptoManager", &info->cryptoManager, get_ManagedObjectReference, 0 },
};

int put_HostConfigManager(struct soap *soap, char *tag, void **ptr, int req);

#endif
