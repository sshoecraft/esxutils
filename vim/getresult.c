
#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG 1
#include "ivim.h"
#include "RetrieveProperties.h"

static char *getnext(char *path) {
	static char name[256];
	register char *p;

	if (!strlen(path)) return 0;

	p = strchr(path,'.');
	if (!p) {
		strcpy(name,path);
		*path = 0;
	} else {
		*p = 0;
		strcpy(name,path);
		strcpy(path,p+1);
		dprintf("NEW path: %s\n", path);
	}

	return name;
}

struct propSet *get_result(list nodes, char *path) {
	char local_path[1024], current_path[1024], *p;
	struct propSet *node;
	list current;

	dprintf("path: %s\n", path);

	/* Try to find a full/partial match on pathSet results */
	list_reset(nodes);
	while( (node = list_get_next(nodes)) != 0) {
		dprintf("node: %s, leaf: %d\n", node->name, (node->value ? 1 : 0));
		if (strcmp(path, node->name) == 0) {
			dprintf("returning: %s\n", node->name);
			return node;
		} else if (strstr(path, node->name) != 0 && node->value == 0) {
			p = strstr(path, node->name) + strlen(node->name) + 1;
			dprintf("path: %s, node->name: %s\n", path, node->name);
			dprintf("p: %s\n", p);
			return get_result(node->nodes, p);
		}
	}

	strcpy(local_path, path);
	current = nodes;

	current_path[0] = 0;
	while( (p = getnext(local_path)) != 0) {
		dprintf("local_path: %s, p: %s\n", local_path, p);
		list_reset(current);
		while((node = list_get_next(current)) != 0) {
			dprintf("node: name: %s, value: %s\n", node->name, (node->value ? node->value : ""));
			if (strcmp(p, node->name) == 0) {
				dprintf("found it.\n");
				strcpy(current_path, node->name);
				dprintf("current_path: %s, path: %s\n", current_path, path);
				if (node->value) {
					if (strcmp(current_path,path) == 0) {
						dprintf("returning: %s\n", node->name);
						return node;
					}
				} else {
					return get_result(node->nodes,local_path);
				}
			} else if (strcmp(node->name, path) == 0) {
				dprintf("returning: %s\n", node->name);
				return node;
			}
		}
	}

	return 0;
}

char *get_result_value(list nodes, char *path) {
	struct propSet *set;

	set = get_result(nodes, path);
	if (!set) return 0;
	if (!set->value) return 0;
	return set->value;
}
