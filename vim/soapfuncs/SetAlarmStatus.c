
#include "SetAlarmStatus.h"

#if 0
<SetAlarmStatus xmlns="urn:vim25">
	<_this type="AlarmManager">AlarmManager</_this>
	<alarm type="Alarm">alarm-36</alarm>
	<entity type="Folder">group-d1</entity>
	<status>green</status>
</SetAlarmStatus>
#endif

static int put_req(struct soap *soap, void *arg) {
	struct SetAlarmStatusRequest *req = arg;
	struct mydesc put_dc_desc[] = {
		{ "_this", &req->alarmManager, put_ManagedObjectReference, 1 },
		{ "alarm", &req->alarm, put_ManagedObjectReference, 1 },
		{ "entity", &req->entity, put_ManagedObjectReference, 1 },
		{ "status", &req->status, put_char, 1 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"SetAlarmStatus","xmlns",MYNS,put_dc_desc);
}

static int get_resp(struct soap *soap, void *arg) {
	if (soap_element_begin_in(soap, "SetAlarmStatusResponse", 1, NULL) || soap_element_end_in(soap, "SetAlarmStatusResponse"))
		return 1;
	return SOAP_OK;
}

SOAPFUNCO(SetAlarmStatus,put_req,struct SetAlarmStatusRequest *,get_resp);
