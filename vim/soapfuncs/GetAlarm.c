
#include "GetAlarm.h"

static int put_req(struct soap *soap, void *arg) {
	struct GetAlarmRequest *req = arg;
	struct mydesc put_req_desc[] = {
		{ "_this", &req->alarmManager, put_ManagedObjectReference, 1 },
		{ "entity", &req->entity, put_ManagedObjectReference, 1 },
		{ 0,0,0,0 }
	};

	return soap_send_desc(soap,"GetAlarm","xmlns",MYNS,put_req_desc);
}

static int get_resp(struct soap *soap, void *arg) {
	list results = arg;
	struct ManagedObjectReference *obj;
	char *tag = "GetAlarmResponse";
	int count,r;

	/* must have <tag> */
	if (soap_element_begin_in(soap, tag, 0, 0)) {
		dprintf("soap_element_begin_in(%s) failed\n",tag);
		return 1;
	}
	dprintf("got <%s>\n",tag);

	count = 0;
	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"returnval") == 0) {
		dprintf("peeked element: %s\n", soap->tag);
		recv_MOR(soap,soap->tag,&obj,0);
		list_add(results,obj,0);
#if 0
		soap_element_begin_in(soap, soap->tag, 0, 0);
		soap_element_end_in(soap,soap->tag);
		if (get_taskInfo(soap, "returnval", results))
			goto get_rnt_error;
		count++;
#endif
	}

	/* must have </tag> */
	if (soap_element_end_in(soap, tag)) {
		dprintf("soap_element_end_in(%s) failed\n",tag);
		return 1;
	}
	dprintf("got </%s>\n",tag);

	r = (count > 0 ? 0 : 1);
	dprintf("returning: %d\n",r);
	return r;

//get_rnt_error:
	soap_element_end_in(soap, tag);
	return 1;
}

SOAPFUNCIO(GetAlarm,put_req,struct GetAlarmRequest *,get_resp,list);
