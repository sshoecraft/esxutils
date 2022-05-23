
#ifndef __VIM_ServiceContent_H
#define __VIM_ServiceContent_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "AboutInfo.h"

struct ServiceContent {
        struct ManagedObjectReference *rootFolder;
        struct ManagedObjectReference *propertyCollector;
        struct ManagedObjectReference *viewManager;
        struct AboutInfo *about;
        struct ManagedObjectReference *setting;
        struct ManagedObjectReference *userDirectory;
        struct ManagedObjectReference *sessionManager;
        struct ManagedObjectReference *authorizationManager;
        struct ManagedObjectReference *perfManager;
        struct ManagedObjectReference *scheduledTaskManager;
        struct ManagedObjectReference *alarmManager;
        struct ManagedObjectReference *eventManager;
        struct ManagedObjectReference *taskManager;
        struct ManagedObjectReference *extensionManager;
        struct ManagedObjectReference *customizationSpecManager;
        struct ManagedObjectReference *customFieldsManager;
        struct ManagedObjectReference *accountManager;
        struct ManagedObjectReference *diagnosticManager;
        struct ManagedObjectReference *licenseManager;
        struct ManagedObjectReference *searchIndex;
        struct ManagedObjectReference *fileManager;
        struct ManagedObjectReference *virtualDiskManager;
        struct ManagedObjectReference *virtualizationManager;
        struct ManagedObjectReference *snmpSystem;
        struct ManagedObjectReference *vmProvisioningChecker;
        struct ManagedObjectReference *vmCompatibilityChecker;
        struct ManagedObjectReference *ovfManager;
        struct ManagedObjectReference *ipPoolManager;
        struct ManagedObjectReference *dvSwitchManager;
        struct ManagedObjectReference *hostProfileManager;
        struct ManagedObjectReference *clusterProfileManager;
        struct ManagedObjectReference *complianceManager;
        struct ManagedObjectReference *localizationManager;
};

struct ServiceContent *new_ServiceContent(struct soap *soap);

#endif
