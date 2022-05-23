
#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG 1

#include "soapfuncs.h"

int dpeek(struct soap *soap) {
        int r = 0;

        r = soap_peek_element(soap);
        if (r == 0) dprintf("peeked element: %s\n", soap->tag);
        return r;
}

/* Utils to send/rescv SOAP data */

int soap_send_req(struct soap *soap, char *endpoint, soapfunc_t *func, void *arg) {
	dprintf("sending...\n");
	soap->encodingStyle = NULL;
	soap_begin(soap);
	soap_serializeheader(soap);
	if (soap_connect(soap, endpoint, "urn:vim25/4.0")) {
		dprintf("soap_connect error\n");
		goto error;
	}
	if (soap_envelope_begin_out(soap)
		|| soap_putheader(soap)
		|| soap_body_begin_out(soap)
		|| func(soap,arg)
		|| soap_body_end_out(soap)
		|| soap_envelope_end_out(soap)
		|| soap_end_send(soap)) {
			dprintf("send error.\n");
			goto error;
	}

	dprintf("send done.\n");
	return 0;
error:
	soap_closesock(soap);
	return 1;
}

int soap_recv_reply(struct soap *soap, soapfunc_t *func, void *arg) {
	dprintf("receiving start...\n");
#if 0
	if (soap_begin_recv(soap)
		|| soap_envelope_begin_in(soap)
		|| soap_recv_header(soap)
		|| soap_body_begin_in(soap)) {
			goto recv_error;
	}
	dprintf("calling func...\n");
	if (func(soap,arg)) goto recv_error;
	dprintf("back from func...\n");
	if (soap_peek_element(soap) == SOAP_OK) dprintf("peeked element: %s\n", soap->tag);
	dprintf("receving end...\n");
	if (soap_body_end_in(soap)
		|| soap_envelope_end_in(soap)
		|| soap_end_recv(soap)) {
			goto recv_error;
	}
#else
	if (soap_begin_recv(soap)
		|| soap_envelope_begin_in(soap)
		|| soap_recv_header(soap)
		|| soap_body_begin_in(soap)
		|| (func ? func(soap,arg) : 0)
		|| soap_body_end_in(soap)
		|| soap_envelope_end_in(soap)
		|| soap_end_recv(soap)) {
			goto recv_error;
	}

	dprintf("receive done.\n");
	return soap_closesock(soap);
#endif

recv_error:
	dprintf("receive error.\n");
	soap_closesock(soap);
	return 1;

}

int soap_send_string(struct soap *soap, char *tag, char *str) {
//	dprintf("tag: %s, str: %p\n", tag, str);
	if (!str) return 0;
	dprintf("tag: %s, str: %s\n", tag, (str ? str : "null"));
//			soap_attribute(soap, "xsi:type", "string") ||
	if (soap_element(soap,tag,0,0) ||
			soap_attribute(soap, "xsi:type", "string") ||
			soap_element_start_end_out(soap, 0) ||
			soap_string_out(soap, str, 0) || 
			soap_element_end_out(soap, tag))
		return 1;
	return SOAP_OK;
}

int put_char(struct soap *soap, char *tag, void **ptr, int req) {
	char *str = (ptr ? *ptr : 0);
	dprintf("str: %p, req: %d\n", str, req);
	if (!str && !req) return 0;
	return soap_send_string(soap,tag,str);
}

int soap_recv_string(struct soap *soap, char *tag, char **ptr, int req) {
	char **str;

	/* XXX which is better? soap_string_in or soap_instring? */
//	*ptr = soap_string_in(soap, 0, -1, -1);
	str = soap_instring(soap, tag, 0, "xsd:string", 0, 1, -1, -1);
	dprintf("tag: %s, str: %s\n", tag, (str ? *str : "(null)"));
	if (str) {
		*ptr = *str;
	} else if (req) {
		dprintf("soap_in_string failed\n");
                return 1;
	}
	return 0;
}

int get_char(struct soap *soap, char *tag, void **ptr, int req) {
	return soap_recv_string(soap,tag,(char **)ptr,req);
}

int soap_send_int(struct soap *soap, char *tag, int value) {
//	dprintf("tag: %s, value: %d\n", tag, value);
	if (soap_element(soap,tag,0,0) ||
			soap_attribute(soap, "xsi:type", "int") ||
			soap_element_start_end_out(soap, 0) ||
			soap_string_out(soap, soap_long2s(soap, (long)value), 0) ||
			soap_element_end_out(soap, tag))
		return soap->error;
	return SOAP_OK;
}

int put_int(struct soap *soap, char *tag, void **ptr, int req) {
	int *intp = (int *) ptr;
	if (!intp && !req) return 0;
	return soap_send_int(soap,tag,*intp);
}

int soap_recv_int(struct soap *soap, char *tag, int *value, int req) {
	char **str;
	str = soap_instring(soap, tag, 0, "xsd:string", 0, 1, -1, -1);
	dprintf("tag: %s, str: %s\n", tag, (str ? *str : "(null)"));
	if (!str && req) {
		dprintf("soap_in_string failed\n");
                return 1;
	}
	if (str) *value = atoi(*str);
	return 0;
}

int get_int(struct soap *soap, char *tag, void **ptr, int req) {
	return soap_recv_int(soap,tag,(int *)ptr,req);
}

int soap_send_bool(struct soap *soap, char *tag, bool value) {
	int data;

//	dprintf("tag: %s, value: %d\n", tag, value);
	if (value < 0)
		data = 0;
	else if (value > 0)
		data = 1;
	else
		return 0;
	if (soap_element(soap,tag,0,0) ||
			soap_attribute(soap, "xsi:type", "boolean") ||
			soap_element_start_end_out(soap, 0) ||
			soap_string_out(soap, soap_long2s(soap, (long)data), 0) ||
			soap_element_end_out(soap, tag))
		return soap->error;
	return SOAP_OK;
}

int put_bool(struct soap *soap, char *tag, void **ptr, int req) {
	bool *boolp = (bool *) ptr;
	if (!boolp && !req) return 0;
	return soap_send_bool(soap,tag,*boolp);
}

int soap_recv_bool(struct soap *soap, char *tag, bool *value, int req) {
	char **str;
	str = soap_instring(soap, tag, 0, "xsd:string", 0, 1, -1, -1);
	dprintf("tag: %s, str: %s\n", tag, (str ? *str : "(null)"));
	if (!str && req) {
		dprintf("soap_in_string failed\n");
                return 1;
	}
	dprintf("str: %p\n", str);
	if (str) *value = (strcmp(*str,"true") == 0 ? 1 : 0);
	return 0;
}

int get_bool(struct soap *soap, char *tag, void **ptr, int req) {
	return soap_recv_bool(soap,tag,(bool *)ptr,req);
}


#if 0
int call_send_func(struct mydesc *dp) {
	switch(dp->type) {
	case VIM_TYPE_OBJ:
		r = send_MOR(soap, dp->tag, dp->dest);
		break;
	case VIM_TYPE_STRING:
	case VIM_TYPE_DATETIME:
		r = soap_send_string(soap, dp->tag, dp->dest);
		break;
	case VIM_TYPE_BOOL:
		boolp = dp->dest;
		r = soap_send_bool(soap, dp->tag, *boolp);
		break;
	case VIM_TYPE_INT:
		intp = dp->dest;
		r = soap_send_int(soap, dp->tag, *intp);
		break;
	case VIM_TYPE_FUNC:
		if (!dp->func) {
			printf("error: func type specified yet func not defined!\n");
			exit(1);
		}
		dprintf("calling func...\n");
		r = dp->func(soap, dp->tag, dp->dest, 0);
		break;
	default:
		printf("error: unknown type: %d\n", dp->type);
		return 1;
	}
	dprintf("r: %d, req: %d\n", r, dp->req);
	if (r && req) return 1;
	return 0;
}
#endif

int soap_send_desc(struct soap *soap, char *tag, char *attr, char *type, struct mydesc *desc) {
	struct mydesc *dp;
	int r;
//	char **p;

	dprintf("tag: %s, attr: %p, type: %p, desc: %p\n", tag, attr, type, desc);

	if (tag) {
		if (soap_element(soap,tag,0,0)) return 1;
		if (attr && type) {
			if (soap_attribute(soap, attr, type)) return 1;
		}
		if (soap_element_start_end_out(soap, 0)) return 1;
	}
	for(dp = desc; dp->tag; dp++) {
		dprintf("tag: %s, dest: %p, func: %p\n", dp->tag, dp->dest, dp->func);
#if 0
		p = dp->dest;
		dprintf("*dest: %p\n", *p);
		if (!*p) continue;
#endif
		r = dp->func(soap, dp->tag, dp->dest, dp->req);
	}
	if (tag && soap_element_end_out(soap, tag) != SOAP_OK) return 1;
	return 0;
}

#if 0
int call_recv_func(struct mydesc *dp) {
	int r;

	switch(dp->type) {
	case VIM_TYPE_OBJ:
		r = recv_MOR(soap, dp->tag, dp->dest, dp->req);
		break;
	case VIM_TYPE_STRING:
	case VIM_TYPE_DATETIME:
		r = soap_recv_string(soap, dp->tag, dp->dest, dp->req);
		break;
	case VIM_TYPE_BOOL:
		r = soap_recv_bool(soap, dp->tag, dp->dest, dp->req);
	break;
	case VIM_TYPE_INT:
		r = soap_recv_int(soap, dp->tag, dp->dest, dp->req);
		break;
	case VIM_TYPE_FUNC:
		if (!dp->func) {
			printf("error: func type specified yet func not defined!\n");
		exit(1);
		}
		dprintf("calling func...\n");
		r = dp->func(soap, dp->tag, dp->dest, dp->req);
		break;
	default:
		printf("call_recv_func: unknown type: %d\n", dp->type);
		r = 1;
	}
}
#endif

static struct mydesc *_findtag(struct mydesc *desc, char *tag) {
	register struct mydesc *dp;

	for(dp = desc; dp->tag; dp++) {
		if (strcmp(dp->tag,tag) == 0)
			break;
	}
	dprintf("tag: %s: %sfound.\n", tag, (dp->tag ? "" : "NOT "));
	return (dp->tag ? dp : 0);
}

int soap_recv_desc(struct soap *soap, char *tag, struct mydesc *desc) {
	struct mydesc *dp;
	int r;

	if (tag) dprintf("tag: %s\n", tag);
	dpeek(soap);

	if (tag) {
		if (soap_element_begin_in(soap, tag, 0, 0)) {
			dprintf("soap_element_begin_in(%s) failed\n", tag);
			goto get_desc_error;
		}
		dprintf("got <%s>\n", tag);
	}

	while (soap_peek_element(soap) == SOAP_OK) {
		dprintf("peeked element: %s\n", soap->tag);
		if (tag && strcmp(soap->tag,tag) == 0) break;
		dp = _findtag(desc,soap->tag);
		if (dp) {
			r = dp->func(soap, dp->tag, dp->dest, dp->req);
			dprintf("r: %d, req: %d\n", r, dp->req);
			if (r && dp->req) goto get_desc_error;
		} else {
			dprintf("element: %s not found, slurping...\n",soap->tag);
			soap_element_begin_in(soap, soap->tag, 0, 0);
			soap_element_end_in(soap, soap->tag);
		}
	}

	if (tag) {
		if (soap_element_end_in(soap, tag)) {
			dprintf("soap_element_end_in(%s) failed\n", tag);
			return 1;
		}
		dprintf("got </%s>\n", tag);
	}

	return 0;

get_desc_error:
	soap_element_end_in(soap, tag);
	return 1;
}
