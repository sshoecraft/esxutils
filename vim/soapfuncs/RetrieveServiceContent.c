
#include "RetrieveServiceContent.h"

static int put_sc(struct soap *soap, void *arg) {
	struct ManagedObjectReference si_ref = { "ServiceInstance", "ServiceInstance" };

//	dprintf("sending req...\n");
	if (soap_element(soap,"RetrieveServiceContent",0,0) ||
			soap_attribute(soap, "xmlns", MYNS) ||
			soap_element_start_end_out(soap, 0) ||
			send_MOR(soap, "_this", &si_ref) ||
			soap_element_end_out(soap,"RetrieveServiceContent")) {
		return 1;
	}
	return 0;
}

static int get_sc(struct soap *soap, void *arg) {
	struct ServiceContent *sc = arg;
	struct mydesc sc_desc[] = {
		{ "rootFolder", &sc->rootFolder, get_ManagedObjectReference, 1 },
		{ "propertyCollector", &sc->propertyCollector, get_ManagedObjectReference, 1 },
		{ "viewManager", &sc->viewManager, get_ManagedObjectReference, 0 },
		{ "about", &sc->about, get_AboutInfo, 1 },
		{ "setting", &sc->setting, get_ManagedObjectReference, 0 },
		{ "userDirectory", &sc->userDirectory, get_ManagedObjectReference, 0 },
		{ "sessionManager", &sc->sessionManager, get_ManagedObjectReference, 0 },
		{ "authorizationManager", &sc->authorizationManager, get_ManagedObjectReference, 0 },
		{ "perfManager", &sc->perfManager, get_ManagedObjectReference, 0 },
		{ "scheduledTaskManager", &sc->scheduledTaskManager, get_ManagedObjectReference, 0 },
		{ "alarmManager", &sc->alarmManager, get_ManagedObjectReference, 0 },
		{ "eventManager", &sc->eventManager, get_ManagedObjectReference, 0 },
		{ "taskManager", &sc->taskManager, get_ManagedObjectReference, 0 },
		{ "extensionManager", &sc->extensionManager, get_ManagedObjectReference, 0 },
		{ "customizationSpecManager", &sc->customizationSpecManager, get_ManagedObjectReference, 0 },
		{ "customFieldsManager", &sc->customFieldsManager, get_ManagedObjectReference, 0 },
		{ "accountManager", &sc->accountManager, get_ManagedObjectReference, 0 },
		{ "diagnosticManager", &sc->diagnosticManager, get_ManagedObjectReference, 0 },
		{ "licenseManager", &sc->licenseManager, get_ManagedObjectReference, 0 },
		{ "searchIndex", &sc->searchIndex, get_ManagedObjectReference, 0 },
		{ "fileManager", &sc->fileManager, get_ManagedObjectReference, 0 },
		{ "virtualDiskManager", &sc->virtualDiskManager, get_ManagedObjectReference, 0 },
		{ "virtualizationManager", &sc->virtualizationManager, get_ManagedObjectReference, 0 },
		{ "snmpSystem", &sc->snmpSystem, get_ManagedObjectReference, 0 },
		{ "vmProvisioningChecker", &sc->vmProvisioningChecker, get_ManagedObjectReference, 0 },
		{ "vmCompatibilityChecker", &sc->vmCompatibilityChecker, get_ManagedObjectReference, 0 },
		{ "ovfManager", &sc->ovfManager, get_ManagedObjectReference, 0 },
		{ "ipPoolManager", &sc->ipPoolManager, get_ManagedObjectReference, 0 },
		{ "dvSwitchManager", &sc->dvSwitchManager, get_ManagedObjectReference, 0 },
		{ "hostProfileManager", &sc->hostProfileManager, get_ManagedObjectReference, 0 },
		{ "clusterProfileManager", &sc->clusterProfileManager, get_ManagedObjectReference, 0 },
		{ "complianceManager", &sc->complianceManager, get_ManagedObjectReference, 0 },
		{ "localizationManager", &sc->localizationManager, get_ManagedObjectReference, 0 },
		{ 0,0,0,0 }
	};

//	dprintf("getting resp...\n");
	if (!sc) return 1;
//	memset(sc,0,sizeof(*sc));
	if (soap_element_begin_in(soap, "RetrieveServiceContentResponse", 0, 0)) {
		dprintf("soap_element_begin_in(RetrieveServiceContentResponse) failed\n");
		return 1;
	}
	if (soap_element_begin_in(soap, "returnval", 0, 0)) {
		dprintf("soap_element_begin_in(returnval) failed\n");
		return 1;
	}
	if (soap_recv_desc(soap, 0, sc_desc)) {
		dprintf("soap_recv_desc failed\n");
		return 1;
	}
	if (soap_element_end_in(soap, "returnval")) {
		dprintf("soap_element_end_in(returnval) failed\n");
		return 1;
	}
	if (soap_element_end_in(soap, "RetrieveServiceContentResponse")) {
		dprintf("soap_element_end_in(RetrieveServiceContentResponse) failed\n");
		return 1;
	}
	return 0;
}

SOAPFUNCI(RetrieveServiceContent,put_sc,get_sc,struct ServiceContent *);
