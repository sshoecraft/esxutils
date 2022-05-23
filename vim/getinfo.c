
#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG 1

#include "ivim.h"
#include "ServiceContent.h"
#include "RetrieveProperties.h"

#ifdef DEBUG
static void dispnodes(list nodes, int level) {
	struct propSet *set;
	int x;

//	dprintf("level: %d, nodes: %d\n", level, list_count(nodes));
	list_reset(nodes);
	while((set = list_get_next(nodes)) != 0) {
		if (!set->name) continue;
		for(x=0; x < level; x++) printf("  ");
		printf("%s", set->name);
		if (set->value)
			printf(": %s\n", set->value);
		else {
			if (!list_count(set->nodes)) {
				printf("\n**** NO SUBS *****\n");
				exit(1);
			}
			printf("\n");
			dispnodes(set->nodes, level+1);
		}
	}
}
#endif

#define ADDNAME(l,n) list_add(l,n.name,strlen(n.name)+1)

static int setfilter(struct PropertyFilterSpec *filter, struct ManagedObjectReference *mo_ref, struct PropertySpec *property_spec) {
	struct TraversalSpec resourcePoolTraversalSpec;
	struct TraversalSpec resourcePoolVmTraversalSpec;
	struct TraversalSpec computeResourceRpTraversalSpec;
	struct TraversalSpec computeResourceHostTraversalSpec;
	struct TraversalSpec cr2ds;
	struct TraversalSpec ccr2ds;
	struct TraversalSpec datacenterFolderTraversalSpec;
	struct TraversalSpec datacenterHostTraversalSpec;
	struct TraversalSpec datacenterVmTraversalSpec;
	struct TraversalSpec dc2ds;
	struct TraversalSpec hostVmTraversalSpec;
	struct TraversalSpec folderTraversalSpec;
	struct ObjectSpec obj_spec;

	resourcePoolTraversalSpec.name = "resourcePoolTraversalSpec";
	resourcePoolTraversalSpec.type = "ResourcePool";
	resourcePoolTraversalSpec.path = "resourcePool";
	resourcePoolTraversalSpec.skip = 0;
	resourcePoolTraversalSpec.selectSet = list_create();
	ADDNAME(resourcePoolTraversalSpec.selectSet, resourcePoolTraversalSpec);

	resourcePoolVmTraversalSpec.name = "resourcePoolVmTraversalSpec";
	resourcePoolVmTraversalSpec.type = "ResourcePool";
	resourcePoolVmTraversalSpec.path = "vm";
	resourcePoolVmTraversalSpec.skip = 0;
	resourcePoolVmTraversalSpec.selectSet = list_create();
	ADDNAME(resourcePoolTraversalSpec.selectSet, resourcePoolVmTraversalSpec);

	computeResourceRpTraversalSpec.name = "computeResourceRpTraversalSpec";
	computeResourceRpTraversalSpec.type = "ComputeResource";
	computeResourceRpTraversalSpec.path = "resourcePool";
	computeResourceRpTraversalSpec.skip = 0;
	computeResourceRpTraversalSpec.selectSet = list_create();
	ADDNAME(computeResourceRpTraversalSpec.selectSet, resourcePoolTraversalSpec);
	ADDNAME(computeResourceRpTraversalSpec.selectSet, resourcePoolVmTraversalSpec);

	computeResourceHostTraversalSpec.name = "computeResourceHostTraversalSpec";
	computeResourceHostTraversalSpec.type = "ComputeResource";
	computeResourceHostTraversalSpec.path = "host";
	computeResourceHostTraversalSpec.skip = 0;
	computeResourceHostTraversalSpec.selectSet = list_create();

	cr2ds.name = "cr2ds";
	cr2ds.type = "ComputeResource";
	cr2ds.path = "datastore";
	cr2ds.skip = 0;
	cr2ds.selectSet = list_create();
	
	ccr2ds.name = "ccr2ds";
	ccr2ds.type = "ClusterComputeResource";
	ccr2ds.path = "datastore";
	ccr2ds.skip = 0;
	ccr2ds.selectSet = list_create();
	
	datacenterFolderTraversalSpec.name = "datacenterHostTraversalSpec";
	datacenterFolderTraversalSpec.type = "Datacenter";
	datacenterFolderTraversalSpec.path = "Folder";
	datacenterFolderTraversalSpec.skip = 0;
	datacenterFolderTraversalSpec.selectSet = list_create();

	datacenterHostTraversalSpec.name = "datacenterHostTraversalSpec";
	datacenterHostTraversalSpec.type = "Datacenter";
	datacenterHostTraversalSpec.path = "hostFolder";
	datacenterHostTraversalSpec.skip = 0;
	datacenterHostTraversalSpec.selectSet = list_create();

	datacenterVmTraversalSpec.name = "datacenterVmTraversalSpec";
	datacenterVmTraversalSpec.type = "Datacenter";
	datacenterVmTraversalSpec.path = "vmFolder";
	datacenterVmTraversalSpec.skip = 0;
	datacenterVmTraversalSpec.selectSet = list_create();

	dc2ds.name = "dc2ds";
	dc2ds.type = "Datacenter";
	dc2ds.path = "datastore";
	dc2ds.skip = 0;
	dc2ds.selectSet = list_create();

	hostVmTraversalSpec.name = "hostVmTraversalSpec";
	hostVmTraversalSpec.type = "HostSystem";
	hostVmTraversalSpec.path = "vm";
	hostVmTraversalSpec.skip = 0;
	hostVmTraversalSpec.selectSet = list_create();

	folderTraversalSpec.name = "folderTraversalSpec";
	folderTraversalSpec.type = "Folder";
	folderTraversalSpec.path = "childEntity";
	folderTraversalSpec.skip = 0;
	folderTraversalSpec.selectSet = list_create();

	ADDNAME(datacenterHostTraversalSpec.selectSet, folderTraversalSpec);
	ADDNAME(datacenterVmTraversalSpec.selectSet, folderTraversalSpec);
	ADDNAME(hostVmTraversalSpec.selectSet, folderTraversalSpec);

	ADDNAME(folderTraversalSpec.selectSet, folderTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, datacenterFolderTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, datacenterHostTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, datacenterVmTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, dc2ds);
	ADDNAME(folderTraversalSpec.selectSet, computeResourceRpTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, computeResourceHostTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, cr2ds);
	ADDNAME(folderTraversalSpec.selectSet, ccr2ds);
	ADDNAME(folderTraversalSpec.selectSet, hostVmTraversalSpec);
	ADDNAME(folderTraversalSpec.selectSet, resourcePoolVmTraversalSpec);

	obj_spec.obj = mo_ref;
	obj_spec.skip = 0;
	obj_spec.selectSet = list_create();
	list_add(obj_spec.selectSet, &folderTraversalSpec, sizeof(folderTraversalSpec));
	list_add(obj_spec.selectSet, &dc2ds, sizeof(dc2ds));
	list_add(obj_spec.selectSet, &datacenterVmTraversalSpec, sizeof(datacenterVmTraversalSpec));
	list_add(obj_spec.selectSet, &datacenterHostTraversalSpec, sizeof(datacenterHostTraversalSpec));
	list_add(obj_spec.selectSet, &cr2ds, sizeof(cr2ds));
	list_add(obj_spec.selectSet, &ccr2ds, sizeof(ccr2ds));
	list_add(obj_spec.selectSet, &computeResourceHostTraversalSpec, sizeof(computeResourceHostTraversalSpec));
	list_add(obj_spec.selectSet, &computeResourceRpTraversalSpec, sizeof(computeResourceRpTraversalSpec));
	list_add(obj_spec.selectSet, &resourcePoolTraversalSpec, sizeof(resourcePoolTraversalSpec));
	list_add(obj_spec.selectSet, &hostVmTraversalSpec, sizeof(hostVmTraversalSpec));
	list_add(obj_spec.selectSet, &resourcePoolVmTraversalSpec, sizeof(resourcePoolVmTraversalSpec));

	filter->propSet = list_create();
	list_add(filter->propSet, property_spec, sizeof(*property_spec));
	filter->objectSet = list_create();
	list_add(filter->objectSet, &obj_spec, sizeof(obj_spec));

	return 0;
}

list vim_getinfo(struct vim_session *s, char *type, char **paths, int all, struct ManagedObjectReference *mo_ref) {
	struct PropertySpec prop_spec;
	struct ObjectSpec obj_spec;
	struct PropertyFilterSpec filter;
	struct RetrievePropertiesRequest req;
	char **p;
	list results;

	dprintf("type: %p, paths: %p, mo_ref: %p\n", type, paths, mo_ref);

	if (!type && mo_ref) type = mo_ref->type;

	prop_spec.type = type;
	prop_spec.all = all;
//	prop_spec.all = (paths ? 0 : 1);
	prop_spec.pathSet = list_create();
	if (paths) {
		for(p = paths; *p; p++) {
//			dprintf("p: %s\n", *p);
			list_add(prop_spec.pathSet,*p,strlen(*p)+1);
		}
	}
//	if (list_count(prop_spec.pathSet) == 0) prop_spec.all = 1;

	/* If we specify both paths and an object, make a direct (non-traversal req) */
	dprintf("paths: %p, mo_ref: %p\n", paths, mo_ref);
	if (paths && mo_ref) {
		obj_spec.obj = mo_ref;
		obj_spec.skip = 0;
		obj_spec.selectSet = list_create();

		filter.propSet = list_create();
		list_add(filter.propSet, &prop_spec, sizeof(prop_spec));
		filter.objectSet = list_create();
		list_add(filter.objectSet, &obj_spec, sizeof(obj_spec));
	} else
		setfilter(&filter,(mo_ref ? mo_ref : s->sc->rootFolder),&prop_spec);

	req.propertyCollector = s->sc->propertyCollector;
	req.filter = &filter;

	results = list_create();

	/* Get the refs */
	if (RetrieveProperties(s->soap,s->endpoint,&req,results)) return 0;

	return results;
}

struct ManagedObjectReference *vim_findobj(struct vim_session *vim, char *type, char *name) {
	list results;
	char *paths[] = {
		"name",
		0
	};
	int count;
	struct returnval *ret;
	struct propSet *set;

	dprintf("type: %s, name: %s\n", type, name);

	results = vim_getinfo(vim, type, paths, 0, 0);
	count = list_count(results);
	dprintf("count: %d\n", count);
	if (count < 1) {
		dprintf("no results?\n");
		return 0;
	}

	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
//		dprintf("ret->obj: %s:%s\n", ret->obj.type, ret->obj.value);
		list_reset(ret->nodes);
		while((set = list_get_next(ret->nodes)) != 0) {
			dprintf("set->name: %s\n", set->name);
			dprintf("set->value: %s\n", set->value);
			if (strcmp(set->name,"name") == 0 && strcmp(set->value,name) == 0) {
				struct ManagedObjectReference *obj;

				dprintf("found.\n");
				obj = malloc(sizeof(struct ManagedObjectReference));
				obj->type = ret->obj.type;
				obj->value = ret->obj.value;
				list_destroy(results);
				return obj;
			}
		}
	}
	dprintf("NOT found.\n");
	list_destroy(results);
	return 0;
}
