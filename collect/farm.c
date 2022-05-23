
#include "collect.h"
#include "gendb_farms.h"
#include <ctype.h>

static char **farm_paths;

static struct farm *add_farm(list farms, struct farm *new_farm) {
//	char temp[64], *p;
	struct farm *farm;

	list_reset(farms);
	while((farm = list_get_next(farms)) != 0) {
		if (strcmp(farm->object,new_farm->object) == 0) {
			return 0;
		}
	}

#if 0
	/* Fixup name */
	dprintf("name: %s\n", new_farm->name);
	strcpy(temp, new_farm->name);
	p = strchr(temp,'_');
	if (p) *p = 0;
	p = strchr(temp,' ');
	if (p) *p = 0;
	p = strchr(temp,'(');
	if (p) *p = 0;
	dprintf("temp: %s\n", temp);
//	if (strcmp(temp,new_farm->name) != 0) exit(1);
	strcpy(new_farm->name,temp);
#endif

	if (strcmp(new_farm->name,"host") == 0) new_farm->site_id = 0;
	dprintf("adding farm: %s, site: %d\n", new_farm->name, new_farm->site_id);
	farm = list_add(farms, new_farm, sizeof(*new_farm));

	return farm;
}

int get_farm_info(struct csession *s, struct farm *farm, struct returnval *ret, int site_hint) {
//	char *name, *p;
//	int site_id;
	struct vim_res2desc farm_desc[] = {
		{ "name", VIM_TYPE_STRING, farm->name, sizeof(farm->name)-1, 0, 1 },
		{ "parent", VIM_TYPE_UNK, 0, 0, 0, 1 },
		{ 0, 0, 0, 0, 0, 0 }
	};

	if (s == 0 && farm == 0 && ret == 0) {
		farm_paths = build_paths(farm_desc);
		return (farm_paths ? 0 : 1);
	}

	memset(farm,0,sizeof(*farm));
	sprintf(farm->object,"%s:%s", ret->obj.type, ret->obj.value);
	strcpy(farm->server, s->vim->server);
	vim_results2desc(s->vim,ret->nodes,farm_desc);

#if IS_HP
	/* Get the site from the name */
	if (strncmp(farm->name,"D3",2) == 0)
		farm->site_id = 7;
	else if (strncmp(farm->name,"V1",2) == 0)
		farm->site_id = 3;
	else {
		char temp[8];

		if (strncmp(farm->name,"Site",4) == 0) {
			strcpy(temp,farm->name + 5);
			farm->site_id = atoi(temp);
		} else if (isdigit(farm->name[1])) {
			temp[0] = farm->name[1];
			temp[1] = 0;
			farm->site_id = atoi(temp);
		} else
			farm->site_id = 0;
	}
#else
	if (strncmp(farm->name,"DDC",3) == 0 || strncmp(farm->name,"ddc",3) == 0 || strncmp(farm->name,"usod",4) == 0)
		farm->site_id = 1;
	else if (strncmp(farm->name,"AUS",3) == 0 || strncmp(farm->name,"txau",4) == 0)
		farm->site_id = 2;
	else if (strncmp(farm->name,"LWV",3) == 0 || strncmp(farm->name,"txlw",4) == 0)
		farm->site_id = 3;
	else if (strncmp(farm->name,"USOW",4) == 0 || strncmp(farm->name,"usow",4) == 0)
		farm->site_id = 5;
	else if (strncmp(farm->name,"NDH",3) == 0 || strncmp(farm->name,"uson",4) == 0)
		farm->site_id = 6;
	else if (strncmp(farm->name,"DMZ",3) == 0)
		farm->site_id = 7;
	else
		farm->site_id = 4;
#endif

	if (farm->site_id == 0 && site_hint > 0) farm->site_id = site_hint;
	dprintf("name: %s, site_id: %d\n", farm->name, farm->site_id);

	return 0;
}

int init_farms(struct csession *s) {
	return get_farm_info(0,0,0,0);
}

static char *get_parent_name(struct csession *s, struct ManagedObjectReference *mo_ref) {
	char *paths[] = { "name", 0 };
	struct returnval *ret;
	list results;
	char *p;

	/* Get name from parent */
	results = vim_getinfo(s->vim, mo_ref->type, paths, (paths[0] ? 0 : 1), mo_ref);
	if (list_count(results) != 1) {
		dprintf("get_farm: parent of parent result count != 1!\n");
		return 0;
	}
	ret = results->first->item;
	if (!ret->nodes) {
		printf("get_farm: no results for parent of parent\n");
		return 0;
	}
	p = get_result_value(ret->nodes,"name");
	if (!p) {
		printf("get_farm: unable to get parent of parent name\n");
		return 0;
	}
	dprintf("parent of parent name: %s\n", p);

	list_destroy(results);
	return p;
}

struct farm *get_farm(struct csession *s, char *object) {
	struct farm *farm, new_farm;
	struct ManagedObjectReference *mo_ref;
	list results;
	struct returnval *ret;
	struct propSet *set;
	char *name;
	int site_hint;

	dprintf("object: %s\n", object);

	list_reset(s->farms);
	while((farm = list_get_next(s->farms)) != 0) {
		if (strcmp(farm->object,object) == 0) {
			dprintf("found.\n");
			return farm;
		}
	}
	dprintf("not found.\n");

	/* Get the info for the object from the server */
	mo_ref = str2mor(object);
	results = vim_getinfo(s->vim, mo_ref->type, farm_paths, 0, mo_ref);
	if (list_count(results) != 1) {
		dprintf("get_farm: count != 1 for: %s\n", object);
		strcpy(new_farm.name,"unknown");
		strcpy(new_farm.server, s->vim->server);
		strcpy(new_farm.object,object);
		return add_farm(s->farms, &new_farm);
	}
	ret = results->first->item;

	dprintf("type: %s\n", ret->obj.type);

	/* If the object's type is "ComputeResource" we need to go up to get name */
	/* We want the datastore info, etc from "ComputeResource" and only the name from the parent */
	name = 0;
	if (strcmp(mo_ref->type,"ComputeResource")==0) {
		struct ManagedObjectReference *parent_ref;

		set = get_result(ret->nodes, "parent");
		if (!set) {
			printf("get_farm: unable to get parent\n");
			return 0;
		}
		parent_ref = set2mor(set);
		name = get_parent_name(s, parent_ref);
		free_mor(parent_ref);
	} else if (strcmp(mo_ref->type,"ClusterComputeResource") != 0 && strcmp(mo_ref->type,"Folder") != 0) {
		printf("get_farm: mo_ref(%s) is not ClusterComputeResource or Folder\n", mo_ref->type);
		return 0;
	}

#if IS_HP
	if (strstr(s->vim->server,"vcheoclab"))
		site_hint = 7;
	else
		site_hint = 0;
#else
	site_hint = 0;
#endif

	if (get_farm_info(s,&new_farm,ret,site_hint))
		return 0;

	if (name) strcpy(new_farm.name,name); 

	return add_farm(s->farms, &new_farm);
}

int get_farms(struct csession *s) {
	struct farm new_farm;
	struct returnval *ret;
	list results;
	int site_hint;

	results =  vim_getinfo(s->vim, "ClusterComputeResource", farm_paths, 0, 0);
	dprintf("result count: %d\n", list_count(results));
	if (list_count(results) < 1) {
		/* XXX not an error */
		return 0;
	}

	if (strstr(s->vim->server,"vcheoclab"))
		site_hint = 7;
	else
		site_hint = 0;

	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
		if (get_farm_info(s,&new_farm,ret,site_hint))
			return 1;
		add_farm(s->farms, &new_farm);
	}

	return 0;
}

static void farm2rec(struct farms_record *rec, struct farm *farm) {
//	memset(rec,0,sizeof(*rec));
	rec->site_id = farm->site_id;
	strncpy(rec->server, farm->server, sizeof(rec->server)-1);
	strncpy(rec->name, farm->name, sizeof(rec->name)-1);
}

int update_farms(struct csession *s) {
	struct farms_record rec;
	struct farm *farm;
	char clause[256];

	list_reset(s->farms);
	while((farm = list_get_next(s->farms)) != 0) {
		dprintf("farm: %s\n", farm->name);
		sprintf(clause," WHERE server = '%s' AND name = '%s'", farm->server, farm->name);
		if (farms_select_record(s->db, &rec, clause)) {
			farm2rec(&rec,farm);
			if (farms_insert(s->db,&rec)) return 1;
			/* Re-select for ID */
			farms_select_record(s->db,&rec,clause);
		} else {
			farm2rec(&rec,farm);
		}
		/* Make sure last_seen is set */
		sprintf(clause,",last_seen = CURRENT_TIMESTAMP WHERE id = %d", rec.id);
		if (farms_update_record(s->db,&rec,clause)) return 1;
		farm->id = rec.id;
        }
	return 0;
}
