
#include "ivim.h"
#include <stdlib.h>
#include <regex.h>
#include "soapfuncs/RetrieveProperties.h"

int match(char *string, char *pattern) {
	int status;
	regex_t re;

	dprintf("string: %s, pattern: %s\n", string, pattern);

	if (regcomp(&re, pattern, REG_EXTENDED|REG_NOSUB) != 0) {
		dprintf("error\n");
		return 0;
	}
	status = regexec(&re, string, (size_t) 0, NULL, 0);
	regfree(&re);
	dprintf("%smatch\n", (status == 0 ? "" : "no "));
	return (status == 0);
}

char **build_paths(struct vim_res2desc *desc) {
	struct vim_res2desc *dp;
	char **paths;
	int count,size;

	count = 0;
	dprintf("desc: %p\n", desc);
	for(dp = desc; dp->path; dp++) count++;
	dprintf("count: %d\n", count);
	size = (count + 1) * sizeof(char *);
	dprintf("size: %d\n", size);
	paths = malloc(size);
	dprintf("paths: %p\n", paths);
	if (!paths) {
		perror("malloc paths");
		return 0;
	}
	count = 0;
	for(dp = desc; dp->path; dp++) {
		dprintf("path: %s\n", dp->path);
		paths[count++] = dp->path;
	}
	dprintf("count: %d\n", count);
	paths[count] = 0;

	return paths;
}

struct ManagedObjectReference *alloc_mor(int ts, int vs) {
	struct ManagedObjectReference *mo_ref;

	mo_ref = malloc(sizeof(*mo_ref));
	mo_ref->type = malloc(ts);
	mo_ref->value = malloc(vs);

	return mo_ref;
}

struct ManagedObjectReference *str2mor(char *in_object) {
	struct ManagedObjectReference *mo_ref;
	char object[64],type[64],value[64];
	register char *p;

	strcpy(object, in_object);
	p = strchr(object,':');
	if (!p) return 0;
	*p++ = 0;
	strcpy(type, object);
	strcpy(value, p);
	dprintf("type: %s, value: %s\n", type, value);
	mo_ref = alloc_mor(strlen(type)+1, strlen(value)+1);
	strcpy(mo_ref->type, type);
	strcpy(mo_ref->value, value);

	return mo_ref;
}

struct ManagedObjectReference *set2mor(struct propSet *set) {
	struct ManagedObjectReference *mo_ref;

	mo_ref = alloc_mor(strlen(set->type)+1, strlen(set->value)+1);
	strcpy(mo_ref->type, set->type);
	strcpy(mo_ref->value, set->value);
	dprintf("type: %s, value: %s\n", mo_ref->type, mo_ref->value);

	return mo_ref;
}

void free_mor(struct ManagedObjectReference *mo_ref) {
	free(mo_ref->type);
	free(mo_ref->value);
	free(mo_ref);
}
