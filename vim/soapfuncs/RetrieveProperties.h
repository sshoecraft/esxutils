
#ifndef __VIM_RetrieveProperties_H
#define __VIM_RetrieveProperties_H

#include "soapfuncs.h"
#include "list.h"
#include "ManagedObjectReference.h"

struct TraversalSpec {
	char *name;
	char *type;
	char *path;
	int skip;
	list selectSet;			/* list of strings (<name>) */
};

struct ObjectSpec {
	struct ManagedObjectReference *obj;
	int skip;
	list selectSet;			/* list of struct TraversalSpec */
};

struct PropertySpec {
	char *type;
	unsigned char all;
	list pathSet;			/* list of strings (<path>) */
};

struct PropertyFilterSpec {
	list propSet;			/* list of struct PropertySpec */
	list objectSet;			/* list of struct ObjectSpec */
};

struct RetrievePropertiesRequest {
	struct ManagedObjectReference *propertyCollector;
	struct PropertyFilterSpec *filter;
};

struct propSet {
	char *name;			/* name */
	char *type;			/* type (if present) */
	char *value;			/* value */
	struct propSet *parent;		/* parent */
	list nodes;			/* children */
};

struct returnval {
	struct ManagedObjectReference obj;
	list nodes;			/* list of propSet */
};

SOAPFUNCDEFIO(RetrieveProperties,struct RetrievePropertiesRequest *,list);

#define dispset(n,s) printf("%s: name: %s, type: %s, value: %s, parent: %p, subs: %p\n", n, (s)->name, (s)->type, (s)->value, (s)->parent, (s)->nodes);

#endif
