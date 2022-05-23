
#include "collect.h"
#include "gendb_datastores.h"

static char **ds_paths;

static struct datastore *add_datastore(list datastores, struct datastore *new_ds) {
	struct datastore *datastore;

	list_reset(datastores);
	while((datastore = list_get_next(datastores)) != 0) {
		if (strcmp(datastore->object,new_ds->object) == 0)
			return 0;
	}
	dprintf("add_datastore: adding datastore: %s\n", new_ds->name);
	return list_add(datastores, new_ds, sizeof(*new_ds));
}

static int get_datastore_info(struct csession *s, struct datastore *new_ds, struct returnval *ret) {
	char *p, *uuid, *name;
	int total, free;
	struct vim_res2desc ds_desc[] = {
		{ 0, 0, 0, 0, 0, 0 }
	};

	if (new_ds == 0 && ret == 0) {
		ds_paths = build_paths(ds_desc);
		return (ds_paths ? 0 : 1);
	}

	dprintf("object: %s:%s\n", ret->obj.type, ret->obj.value);

	memset(new_ds,0,sizeof(*new_ds));
	p = get_result_value(ret->nodes, "info.vmfs.uuid");
	if (!p) p = get_result_value(ret->nodes, "info.url");
	if (!p) {
		printf("get_datastore_info: unable to get uuid or URL from %s:%s\n", ret->obj.type, ret->obj.value);
		return 1;
	}
	uuid = p;
	name = GETVAL("info.name");
	GETVAL("summary.capacity");
	total = (p ? atoll(p) / 1048576 : 0);
	GETVAL("summary.freeSpace");
	free = (p ? atoll(p) / 1048576 : 0);

	p = get_result_value(ret->nodes, "info.vmfs.blockSizeMb");
	new_ds->blocksize = (p ? atoll(p) : 0);

	sprintf(new_ds->object, "%s:%s", ret->obj.type, ret->obj.value);
	strcpy(new_ds->uuid, uuid);
	if (name) strcpy(new_ds->name, name);
	new_ds->total = total;
	new_ds->free = free;

	return 0;
}

int init_datastores(struct csession *s) {
        return get_datastore_info(0,0,0);
}

struct datastore *get_datastore(struct csession *s, char *object) {
	struct datastore *ds, new_ds;
	char *paths[] = {
		"info",
		0
	};
	struct ManagedObjectReference *mo_ref;
	list results;
	struct returnval *ret;

	dprintf("object: %s\n", object);

	/* Find it */
	list_reset(s->datastores);
	while((ds = list_get_next(s->datastores)) != 0) {
		if (strcmp(ds->object,object) == 0) {
			dprintf("found. ds: %p\n", ds);
			return ds;
		}
	}
	dprintf("not found.\n");

	/* Not found, ask for it */
	mo_ref = str2mor(object);
	results = vim_getinfo(s->vim, mo_ref->type, paths, 0, mo_ref);
	if (list_count(results) != 1) {
		printf("get_datastore(%s:%s): count != 1\n", mo_ref->type, mo_ref->value);
		return 0;
	}
	ret = results->first->item;
	get_datastore_info(s, &new_ds, ret);
	ds = add_datastore(s->datastores, &new_ds);
	free_mor(mo_ref);
	return ds;
}

int get_datastores(struct csession *s) {
	struct datastore new_ds;
	char *paths[] = {
		"info",
		"summary",
		0
	};
	struct returnval *ret;
	list results;

	results = vim_getinfo(s->vim, "Datastore", paths, 0, 0);
	dprintf("result count: %d\n", list_count(results));
	if (list_count(results) < 1) {
		/* XXX not an error */
		return 0;
	}

	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
		if (get_datastore_info(s, &new_ds, ret)) 
			return 1;
		add_datastore(s->datastores, &new_ds);
	}

	return 0;
}

static void datastore2rec(struct datastores_record *rec, struct datastore *ds) {
//	memset(rec,0,sizeof(*rec));
	strcpy(rec->uuid, ds->uuid);
	strcpy(rec->name, ds->name);
	rec->blocksize = ds->blocksize;
	rec->total = ds->total;
	rec->free = ds->free;
}

int update_datastores(struct csession *s) {
	struct datastores_record rec;
	struct datastore *ds;
	char clause[64];

	dprintf("s: %p\n",s);
	if (!s) return 1;
	dprintf("s->datastores: %p\n",s->datastores);
	if (!s->datastores) return 1;
	list_reset(s->datastores);
	while((ds = list_get_next(s->datastores)) != 0) {
		dprintf("datastore: %s\n", ds->uuid);
		if (datastores_select_by_uuid(s->db, &rec, ds->uuid)) {
			datastore2rec(&rec,ds);
			if (datastores_insert(s->db,&rec)) return 1;
			datastores_select_by_uuid(s->db,&rec,ds->uuid);
		} else {
			datastore2rec(&rec,ds);
			sprintf(clause,",last_seen = CURRENT_TIMESTAMP WHERE id = %d", rec.id);
			if (datastores_update_record(s->db,&rec,clause)) return 1;
		}
		ds->id = rec.id;
        }
	return 0;
}
