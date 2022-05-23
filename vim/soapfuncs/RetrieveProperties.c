
#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG 1

#include "RetrieveProperties.h"

static int put_propSet(struct soap *soap, list set) {
	struct PropertySpec *prop;

	soap_element_begin_out(soap, "propSet",0,0);
	list_reset(set);
	while( (prop = list_get_next(set)) != 0) {
//		dprintf("type: %s\n", prop->type);
//		soap_send_string(soap, "type", prop->type);
		soap_send_string(soap, "type", prop->type);
//		soap_send_int(soap, "all", prop->all);
		soap_send_int(soap, "all", prop->all);
		if (list_count(prop->pathSet)) {
			char *path;

			list_reset(prop->pathSet);
			while( (path = list_get_next(prop->pathSet)) != 0) {
//				dprintf("path: %s\n", path);
//				soap_send_string(soap, "pathSet", path);
				soap_send_string(soap, "pathSet", path);
			}
		}
	}
	soap_element_end_out(soap, "propSet");

	return 0;
}

static int put_selectSet(struct soap *soap, list set) {
	struct TraversalSpec *tp;

	list_reset(set);
	while( (tp = list_get_next(set)) != 0) {
//		dprintf("name: %s\n", tp->name);
		soap_element_begin_out(soap, "selectSet", 0, "TraversalSpec");
		soap_send_string(soap, "name", tp->name);
		soap_send_string(soap, "type", tp->type);
		soap_send_string(soap, "path", tp->path);
		soap_send_int(soap, "skip", tp->skip);
		if (list_count(tp->selectSet)) {
			char *name;

			list_reset(tp->selectSet);
			while( (name = list_get_next(tp->selectSet)) != 0) {
				soap_element_begin_out(soap, "selectSet", 0, 0);
				soap_send_string(soap, "name", name);
				soap_element_end_out(soap, "selectSet");
			}
		}
		soap_element_end_out(soap, "selectSet");
	}
	return 0;
}

static int put_ManagedObjectReferenceectSet(struct soap *soap, list set) {
	struct ObjectSpec *obj;

	soap_element_begin_out(soap, "objectSet", 0, 0);
	list_reset(set);
	while( (obj = list_get_next(set)) != 0) {
//		dprintf("obj: type: %s, val: %s\n", obj->obj->type, obj->obj->val);
		send_MOR(soap, "obj", obj->obj);
		soap_send_int(soap, "skip", obj->skip);
		if (list_count(obj->selectSet)) put_selectSet(soap, obj->selectSet);
	}
	soap_element_end_out(soap, "objectSet");

	return 0;
}

static int put_prop(struct soap *soap, void *arg) {
	struct RetrievePropertiesRequest *req = arg;

	if (soap_element(soap,"RetrieveProperties",0,0) ||
			soap_attribute(soap, "xmlns", MYNS) ||
			soap_element_start_end_out(soap, 0) ||
			send_MOR(soap,"_this",req->propertyCollector)) {
		return 1;
	}

	soap_element_begin_out(soap,"specSet",0,0);
	dprintf("propSet count: %d\n", list_count(req->filter->propSet));
	if (list_count(req->filter->propSet) > 0 && put_propSet(soap, req->filter->propSet)) return 1;
	dprintf("objectSet count: %d\n", list_count(req->filter->objectSet));
	if (list_count(req->filter->objectSet) > 0 && put_ManagedObjectReferenceectSet(soap, req->filter->objectSet)) return 1;
	soap_element_end_out(soap,"specSet");

	return soap_element_end_out(soap,"RetrieveProperties");
}

static int get_propSetSubs(struct soap *soap, struct propSet *set) {

//	dprintf("set: %p\n", set);
	if (!set) {
		dprintf("\nSET IS NULL\n");
		exit(1);
	}

	set->name = (char *) malloc(strlen(soap->tag)+1);
	strcpy(set->name, soap->tag);

	if (soap_element_begin_in(soap, set->name, 0, 0)) {
		dprintf("soap_element_begin_in(%s) failed\n", set->name);
		return 0;
	}
//	dprintf("got <%s>\n", set->name);

	/* Type specified? */
	set->type = 0;
	if (soap_s2string(soap, soap_attr_value(soap, "type", 0), &set->type, -1, -1)) {
		dprintf("soap_s2string(%s/type) failed\n", set->name);
		goto propsubset_error;
	}
//	if (set->type) dprintf("got type: %s\n", set->type);

	/* could have <tag>, else will have val */
	if (soap_peek_element(soap) == 0) {
		struct propSet subset;

		set->nodes = list_create();
		while(soap_peek_element(soap) == 0) {
			if (get_propSetSubs(soap, &subset))
				goto propsubset_error;
//			dprintf("adding: name: %s, value: %s, subs; %d\n", subset.name, (subset.value ? subset.value : ""),
//				list_count(subset.nodes));
			list_add(set->nodes, &subset, sizeof(subset));
		}
		set->value = 0;
	} else {
		soap_instring(soap, "-", &set->value, "xsd:string", 0, 1, -1, -1);
		set->nodes = 0;
//		dprintf("value: %s\n", set->value);
	}

	if (soap_element_end_in(soap, set->name)) {
		dprintf("soap_element_end_in(%s) failed\n", set->name);
		return 1;
	}
//	dprintf("got </%s>\n", set->name);

	return 0;

propsubset_error:
	free(set->name);
	return 1;
}

static int get_propSet(struct soap *soap, struct propSet *set) {

	/* must have <propSet> */
	if (soap_element_begin_in(soap, "propSet", 0, 0)) {
		dprintf("soap_element_begin_in(propSet) failed\n");
		return 1;
	}
//	dprintf("got <propSet>\n");

	/* must have <name> */
	if (soap_element_begin_in(soap, "name", 0, 0) ||
			!soap_instring(soap, "-", &set->name, "xsd:string", 0, 1, -1, -1) ||
			soap_element_end_in(soap, "name")) {
		dprintf("soap_element_begin_in(name) failed!\n");
		goto propSet_error;
	}
//	dprintf("name: %s\n", set->name);

	/* must have <val */
	if (soap_element_begin_in(soap, "val", 0, 0)) {
		dprintf("soap_element_begin_in(val) failed!\n");
		goto propSet_error;
	}
//	dprintf("got <val \n");

	/* If type is present on the val line, get it */
	if (soap_s2string(soap, soap_attr_value(soap, "type", 0), &set->type, -1, -1)) set->type = 0;
//	else dprintf("got type: %s\n", set->type);

	/* could have <tag>, else will have val */
	if (soap_peek_element(soap) == 0) {
		struct propSet subset;
		
		set->nodes = list_create();
		while(soap_peek_element(soap) == 0) {
			if (get_propSetSubs(soap, &subset))
				goto propSet_error;
//			dprintf("adding: name: %s, value: %s, subs; %d\n", subset.name, (subset.value ? subset.value : ""),
//				list_count(subset.nodes));
			list_add(set->nodes, &subset, sizeof(subset));
		}
		set->value = 0;
	} else {
		soap_instring(soap, "-", &set->value, "xsd:string", 0, 1, -1, -1);
		set->nodes = 0;
//		dprintf("value: %s\n", set->value);
	}

	/* must have </val> */
	if (soap_element_end_in(soap, "val")) {
		dprintf("soap_element_end_in(propSet) failed\n");
		goto propSet_error;
	}
//	dprintf("got </val>\n");

	/* must have </propSet> */
	if (soap_element_end_in(soap, "propSet")) {
		dprintf("soap_element_end_in(propSet) failed\n");
		return 1;
	}
//	dprintf("got </propSet>\n");

	return 0;

propSet_error:
	/* </propSet> */
	soap_element_end_in(soap, "propSet");
	return 1;
}

#if UNIQUE_NAMES
static char *_newname(struct soap *soap, char *name, int num) {
	char val[16], *p;

	sprintf(val, "%d", num);
	p = soap_malloc(soap, strlen(name)+strlen(val)+1);
	if (!p) return 0;
	sprintf(p,"%s%s", name, val);
	soap_dealloc(soap, name);

	return p;
}

static void _chkname(struct soap *soap, struct propSet *set) {
	struct propSet *set2;
	list_item item,item2;
	int n;

	n = 2;
	for(item2 = item->next; item2; item2 = item2->next) {
		set2 = item2->item;
		if (strcmp(set->name, set2->name) == 0) {
			set2->name = _newname(soap, set2->name, n++);
		}
	}
	if (n > 2) set->name = _newname(soap, set->name, 1);
}
#endif

static void _fixup(struct soap *soap, list nodes, struct propSet *parent) {
	struct propSet *set;
	list_item item;
//	struct propSet *set2;
//	list_item item2;
//	int n;

	for(item = nodes->first; item; item = item->next) {
		set = item->item;
		set->parent = parent;
		if (!set->value) _fixup(soap, set->nodes, set);
#if UNIQUE_NAMES
		_chkname(soap, set);
#endif
#if 0
		n = 2;
		for(item2 = item->next; item2; item2 = item2->next) {
			set2 = item2->item;
			if (strcmp(set->name, set2->name) == 0) {
				set2->name = _newname(soap, set2->name, n++);
			}
		}
		if (n > 2) set->name = _newname(soap, set->name, 1);
#endif
	}
}

static int get_returnval(struct soap *soap, list results) {
	struct returnval ret;
	struct propSet set;

	memset(&ret,0,sizeof(ret));

	/* must have <returnval> */
	if (soap_element_begin_in(soap, "returnval", 0, 0)) {
		dprintf("soap_element_begin_in(returnval) failed\n");
		return 1;
	}
//	dprintf("got <returnval>\n");

	/* must have <obj> */
	if (soap_element_begin_in(soap, "obj", 0, 0)
		|| soap_s2string(soap, soap_attr_value(soap, "type", 0), &ret.obj.type, -1, -1)
		|| !soap_instring(soap, "-", &ret.obj.value, "xsd:string", 0, 1, -1, -1)
		|| soap_element_end_in(soap, "obj")) {
		dprintf("error getting obj\n");
		goto get_returnval_error;
	}
//	dprintf("obj: type: %s, value: %s\n", ret.obj.type, ret.obj.value);

	/* could have <propSet> (mult) */
	ret.nodes = list_create();
	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"propSet") == 0) {
		if (get_propSet(soap, &set))
			goto get_returnval_error;
		list_add(ret.nodes, &set, sizeof(set));
	}

	/* must have </returnval> */
	if (soap_element_end_in(soap, "returnval")) {
		dprintf("soap_element_end_in(returnval) failed\n");
		goto get_returnval_error;
	}
//	dprintf("got </returnval>\n");

	/* make names unique */
	_fixup(soap, ret.nodes, 0);

	list_add(results,&ret,sizeof(ret));

	return 0;

get_returnval_error:
	soap_element_end_in(soap, "returnval");
	if (ret.nodes) list_destroy(ret.nodes);
	return 1;
}

static int get_prop(struct soap *soap, void *arg) {
	list results = arg;

	/* must have <RetrievePropertiesResponse> */
	if (soap_element_begin_in(soap, "RetrievePropertiesResponse", 0, 0)) {
		dprintf("soap_element_begin_in(RetrievePropertiesResponse) failed\n");
		return 1;
	}
	dprintf("got <RetrievePropertiesResponse>\n");

	while (soap_peek_element(soap) == SOAP_OK && strcmp(soap->tag,"returnval") == 0) {
		if (get_returnval(soap, results))
			goto prop_error;
	}

	/* must have </RetrievePropertiesResponse> */
	if (soap_element_end_in(soap, "RetrievePropertiesResponse")) {
		dprintf("soap_element_end_in(RetrievePropertiesResponse) failed\n");
		return 1;
	}
	dprintf("got </RetrievePropertiesResponse>\n");

	return 0;

prop_error:
	soap_element_end_in(soap, "RetrievePropertiesResponse");
	return 1;
}

SOAPFUNCIO(RetrieveProperties,put_prop,struct RetrievePropertiesRequest *,get_prop,list);
