
#include <stdio.h>
#include "vim.h"
#include "version.h"

#include "soapfuncs/ManagedObjectReference.h"
#include "soapfuncs/RetrieveProperties.h"
#include "soapfuncs/ServiceContent.h"

#ifdef __WIN32__
#include "getopt.h"
#endif

#define OBJCHAR ':'
char comma, do_vars, do_indent, xmlcomp, disp_mor;

void _dnode(struct propSet *set) {
	char path[1024], *p;
	list_item item;
	list paths;
	char *type, *value;
	int x;

	type = set->type;
	value = set->value;
//	dprintf("type: %s, value: %s\n", type, value);
	paths = list_create();
	while(set) {
		list_add(paths,set->name,strlen(set->name)+1);
		set = set->parent;
	}
	path[0] = 0;
	x = 0;
	for(item = paths->last; item; item = item->prev) {
		if (x++) strcat(path, ".");
		strcat(path, (char *)item->item);
	}
	if (do_vars) {
		for(x=0; path[x]; x++) {
			if (path[x] == '.')
				path[x] = '_';
		}
	}
	if (value) {
//		dprintf("value(1): %s\n", value);
		for(p=value; *p; p++) {
			if (*p == '\'' || *p == '\"') {
				if (*(p+1))
					strcpy(p, p+1);
				else
					*p = 0;
			}
		}
//		dprintf("value(2): %s\n", value);
	}
	if (type && strstr(type,"xsd:") == 0) {
		if (!strlen(value)) {
			if (do_vars)
				printf("%s=\'\'\n", path);
			else
				printf("%s%c(null)\n", path, comma);
		} else {
			if (do_vars) 
				printf("%s=\'%s%c%s\'\n", path, type, OBJCHAR, value);
			else
				printf("%s%c%s%c%s\n", path, comma, type, OBJCHAR, value);
		}
	} else {
		if (do_vars)
			printf("%s=\'%s\'\n", path, (strlen(value) ? value : ""));
		else
#if 0
			printf("%s%c\'%s\'\n", path, comma, (strlen(value) ? value : "(null)"));
#endif
			printf("%s%c%s\n", path, comma, (strlen(value) ? value : "(null)"));
	}
}

static char *_newname(char *name, int num) {
	char val[16], *p;

	sprintf(val, "%d", num);
	p = malloc(strlen(name)+strlen(val)+1);
	if (!p) return 0;
	sprintf(p,"%s%s", name, val);

	return p;
}

static void _fixnames(list nodes) {
	struct propSet *set,*set2;
	list_item item,item2;
	int n;

	if (!nodes) return;
	for(item = nodes->first; item; item = item->next) {
		set = item->item;
		n = 2;
		for(item2 = item->next; item2; item2 = item2->next) {
			set2 = item2->item;
//			dprintf("set->name: %s, set2->name: %s\n", set->name, set2->name);
			if (strcmp(set->name, set2->name) == 0) {
				set2->name = _newname(set2->name, n++);
			}
		}
		if (n > 2) set->name = _newname(set->name, 1);
	}
}

void _disp(list nodes, int level) {
	struct propSet *set;
	int i,first;

	first = 0;
	list_reset(nodes);
	while((set = list_get_next(nodes)) != 0) {
		if (set->nodes) _fixnames(set->nodes);
		if (do_indent) for(i=0; i < level; i++) printf("  ");
		if (!set->value) {
			_disp(set->nodes,(first ? level+1 : level));
		} else {
			_dnode(set);
			first = 1;
		}
	}
}

void tagstart(char *tag, int level) {
	register int i;

	if (!xmlcomp) for(i=0; i < level; i++) printf("  ");
	printf("<%s>%s", tag, (xmlcomp ? "" : "\n"));
}

void tagend(char *tag, int level) {
	register int i;

	if (!xmlcomp) for(i=0; i < level; i++) printf("  ");
	printf("</%s>%s", tag, (xmlcomp ? "" : "\n"));
}

void _dump_xml(struct propSet *set, int level) {
	int i;

	if (set->nodes) {
		struct propSet *sub;

		tagstart(set->name,level);
#if 0
		if (!xmlcomp) for(i=0; i < level; i++) printf("  ");
		printf("<%s>%s", set->name,(xmlcomp ? "" : "\n"));
#endif
		list_reset(set->nodes);
		while((sub = list_get_next(set->nodes)) != 0) {
			_dump_xml(sub, level+1);
		}
#if 0
		if (!xmlcomp)for(i=0; i < level; i++) printf("  ");
		printf("</%s>%s", set->name, (xmlcomp ? "" : "\n"));
#endif
		tagend(set->name,level);
	} else {
		if (!xmlcomp) for(i=0; i < level; i++) printf("  ");
		printf("<%s>%s</%s>%s", set->name, set->value, set->name, (xmlcomp ? "" : "\n"));
	}
	return;
}

struct propSet *_mkpset(char *name, struct propSet *set) {
	struct propSet *pset;

	dprintf("name: %s\n", name);

	pset = malloc(sizeof(*pset));
	pset->name = malloc(strlen(name)+1);
	strcpy(pset->name,name);
	pset->value = 0;
	pset->nodes = list_create();
	list_add(pset->nodes,set,sizeof(*set));

	return pset;
}

struct propSet *_expset(struct propSet *set) {
	struct propSet *nset;
	char name[128],*p;

	strcpy(name,set->name);
	while(1) {
		dprintf("name: %s\n", name);
		p = strrchr(name,'.');
		if (!p) break;
		*p = 0;
		nset = malloc(sizeof(*nset));
		nset->name = malloc(strlen(name)+1);
		strcpy(nset->name,p+1);
		nset->value = set->value;
		nset->nodes = set->nodes;
		set = _mkpset(name,nset);
	}

	dprintf("returning set: %s\n", set->name);
	return set;
}

void dump_xml(list ret_nodes, int level) {
	struct propSet *set;

	list_reset(ret_nodes);
	while((set = list_get_next(ret_nodes)) != 0) {
		_dump_xml(_expset(set),level);
	}
}

int print_sc(struct ServiceContent *sc) {
	struct _pinfo {
		char *name;
		struct ManagedObjectReference *mor;
	} info[] = {
		{ "rootFolder", sc->rootFolder },
		{ "propertyCollector", sc->propertyCollector },
		{ "viewManager", sc->viewManager },
		{ "setting", sc->setting },
		{ "userDirectory", sc->userDirectory },
		{ "sessionManager", sc->sessionManager },
		{ "authorizationManager", sc->authorizationManager },
		{ "perfManager", sc->perfManager },
		{ "scheduledTaskManager", sc->scheduledTaskManager },
		{ "alarmManager", sc->alarmManager },
		{ "eventManager", sc->eventManager },
		{ "taskManager", sc->taskManager },
		{ "extensionManager", sc->extensionManager },
		{ "customizationSpecManager", sc->customizationSpecManager },
		{ "customFieldsManager", sc->customFieldsManager },
		{ "accountManager", sc->accountManager },
		{ "diagnosticManager", sc->diagnosticManager },
		{ "licenseManager", sc->licenseManager },
		{ "searchIndex", sc->searchIndex },
		{ "fileManager", sc->fileManager },
		{ "virtualDiskManager", sc->virtualDiskManager },
		{ "virtualizationManager", sc->virtualizationManager },
		{ "snmpSystem", sc->snmpSystem },
		{ "vmProvisioningChecker", sc->vmProvisioningChecker },
		{ "vmCompatibilityChecker", sc->vmCompatibilityChecker },
		{ "ovfManager", sc->ovfManager },
		{ "ipPoolManager", sc->ipPoolManager },
		{ "dvSwitchManager", sc->dvSwitchManager },
		{ "hostProfileManager", sc->hostProfileManager },
		{ "clusterProfileManager", sc->clusterProfileManager },
		{ "complianceManager", sc->complianceManager },
		{ "localizationManager", sc->localizationManager },
		{ 0,0 }
	};
	struct _pinfo *p;
	char temp[64];

	for(p=info; p->name; p++) {
		if (!p->mor) continue;
		if (!p->mor->type) continue;
		if (!strlen(p->mor->type)) continue;
		sprintf(temp,"%s:", p->name);
		printf("%-25s %s:%s\n", temp, p->mor->type, p->mor->value);
	}

	return 0;
}

int print_perf(struct vim_session *s) {
	struct vim_perfmetric *metric;
	char temp[16];

	if (!s->metrics && vim_getallperfmetrics(s)) return 1;
	if (!list_count(s->metrics)) {
		printf("error: no metrics found!\n");
		return 1;
	}
#define HEADER "%-8.8s %-15.15s %-15.15s %-15.15s %-15.15s\n"
	printf(HEADER,"ID","Name","Group","Unit","Type");
	printf(HEADER,"--------------------","--------------------","--------------------","--------------------","--------------------");
	list_reset(s->metrics);
	while((metric = list_get_next(s->metrics)) != 0) {
//		printf("id: %d\n", metric->id);
		sprintf(temp,"%d",metric->id);
		printf(HEADER,temp,metric->name,metric->group,metric->unit,metric->type);
	}

	return 0;
}

void usage(void) {
	printf("usage: esxconf [options]\n");
	printf("  where options are:\n");
	printf("    -a             AboutInfo from connection\n");
	printf("    -c             comma-delimited output\n");
	printf("    -s             server/port to connect to\n");
	printf("    -u             username\n");
	printf("    -p             password\n");
	printf("    -o             specify object\n");
	printf("    -r             print servicecontent\n");
	printf("    -i             always print object IDs\n");
	printf("    -t             specify type (defaults to HostSystem)\n");
	printf("    -h             this listing\n");
	printf("    -n             no paths\n");
	printf("    -e             output as shell variables\n");
	printf("    -f             force type (when used with obj)\n");
	printf("    -x             dump in xml format\n");
	printf("    -z             compressed xml (no newlines/breakout)\n");
	printf("    -q             print perf metrics\n");
	printf("    -V             display version\n");
	exit(1);
}

int main(int argc, char **argv) {
	struct returnval *ret;
	struct vim_session *s;
	list results;
	struct ManagedObjectReference obj;
	char server[64], vcuser[64], *user, *pass, *type, *p;
	char **paths;
	int port, ch, i, have_obj, no_path, do_sc, force_type, do_xml, level, about;
	int do_perf;
	char *gi_type, **gi_paths;
	struct ManagedObjectReference *gi_obj;
	int (*login_func)(struct vim_session *, char *, char *) = vim_login;
	int prompt;
	char password[32];

	comma = ' ';
	have_obj = no_path = do_sc = do_vars = force_type = port = do_xml = do_indent = xmlcomp = 0;
	disp_mor = about = 0;
	do_perf = 0;
	prompt = 0;
	server[0] = 0;
	user = pass = 0;
	type = "HostSystem";
	while((ch = getopt(argc, argv, "acs:u:p:o:hmnrt:efixzqVP")) != -1) {
		switch(ch) {
		case 'a':
			about = 1;
			break;
		case 'c':
			comma = ',';
			break;
		case 'e':
			do_vars = 1;
			break;
		case 'f':
			force_type = 1;
			break;
		case 'h':
			usage();
			exit(0);
		case 'i':
			do_indent = 1;
			break;
		case 'm':
			disp_mor = 1;
			break;
		case 'n':
			no_path = 1;
			break;
		case 'o':
			p = strchr(optarg,OBJCHAR);
			if (!p) usage();
			dprintf("optarg: %s, p: %s\n", optarg, p);
			i = p - optarg;
			dprintf("i: %d\n", i);
			obj.type = malloc(i+1);
			/* XXX strncpy switched to strncat cause ming fks it up */
			memset(obj.type,0,i+1);
			strncat(obj.type, optarg, i);
			i = strlen(p+1);
			dprintf("i: %d\n", i);
			obj.value = malloc(i+1);
			memset(obj.value,0,i+1);
			strcat(obj.value, p+1);
			dprintf("obj: type: %s, val: %s\n", obj.type, obj.value);
			have_obj = 1;
			break;
		case 'p':
			pass = optarg;
			break;
		case 'q':
			do_perf = 1;
			break;
		case 'r':
			do_sc = 1;
			break;
		case 's':
			server[0] = 0;
			p = strchr(optarg,':');
			if (p) {
				strncat(server, optarg, p - optarg);
				dprintf("port: %s\n", p);
				port = atoi(p+1);
			} else {
				strncat(server, optarg, sizeof(server)-1);
				port = 0;
			}
			dprintf("server: %s\n", server);
			if (strcmp(server,"-")==0) {
				int bytes;

				bytes = read(STDIN_FILENO,server,sizeof(server));
				printf("bytes: %d\n", bytes);
			}
			dprintf("server: %s\n", server);
			break;
		case 't':
			type = optarg;
			break;
		case 'u':
			user = optarg;
			break;
		case 'x':
			do_xml = 1;
			break;
		case 'z':
			xmlcomp = 1;
			break;
		case 'P':
			prompt = 1;
			break;
		case 'V':
			printf("esxconf version %s\n", VERSIONSTR);
			exit(0);
			break;
		case '?':
			usage();
			exit(1);
		}
	}
//	printf("*server: %d, user: %p\n", *server, user);
	if (!server[0]) strcpy(server, "vcserver");
	dprintf("server: %s, port: %d\n", server, port);
        if (prompt) {
                if (vim_getpass(password,sizeof(password))) pass = password;
        }

	paths = 0;
	if (no_path) {
		paths = (char **) malloc(sizeof(char *));
		paths[0] = 0;
	} else {
		dprintf("optind: %d, argc: %d\n", optind, argc);
		if (optind < argc) {
			i = argc - optind;
			dprintf("i: %d\n", i);
			paths = (char **) malloc(sizeof(char *) * (i+1));
			if (!paths) {
				perror("malloc");
				return 1;
			}
			i = 0;
			while(optind < argc) paths[i++] = argv[optind++];
			paths[i] = 0;

			for(i=0; paths[i]; i++) dprintf("path[%d]: %s\n", i, paths[i]);
		}
	} 

	s = vim_connect(server, port);
	if (!s) {
		printf("error connecting to server\n");
		return 1;
	}
	if (about) {
		if (comma == ',') {
			char vmw[32],*p;
			vmw[0] = 0;
			strncat(vmw,s->sc->about->vendor,sizeof(vmw)-1);
			for(p=vmw;*p;p++) { if (*p == ',') *p = ' '; }
			printf("%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s,%s\n",
				s->sc->about->name,
				s->sc->about->fullName,
				vmw,
				s->sc->about->version,
				s->sc->about->build,
				s->sc->about->localeVersion,
				s->sc->about->localeBuild,
				s->sc->about->osType,
				s->sc->about->productLineId,
				s->sc->about->apiType,
				s->sc->about->apiVersion,
				s->sc->about->instanceUuid,
				s->sc->about->licenseProductName,
				s->sc->about->licenseProductVersion
			);
		} else {
#if 0
			printf("name:                  %p\n", s->sc->about->name);
			printf("fullName:              %p\n", s->sc->about->fullName);
			printf("vendor:                %p\n", s->sc->about->vendor);
			printf("version:               %p\n", s->sc->about->version);
			printf("build:                 %p\n", s->sc->about->build);
			printf("localeVersion:         %p\n", s->sc->about->localeVersion);
			printf("localeBuild:           %p\n", s->sc->about->localeBuild);
			printf("osType:                %p\n", s->sc->about->osType);
			printf("productLineId:         %p\n", s->sc->about->productLineId);
			printf("apiType:               %p\n", s->sc->about->apiType);
			printf("apiVersion:            %p\n", s->sc->about->apiVersion);
			printf("instanceUuid:          %p\n", s->sc->about->instanceUuid);
			printf("licenseProductName:    %p\n", s->sc->about->licenseProductName);
			printf("licenseProductVersion: %p\n", s->sc->about->licenseProductVersion);
#endif
			printf("AboutInfo:\n");
			printf("name:                  %s\n", s->sc->about->name);
			printf("fullName:              %s\n", s->sc->about->fullName);
			printf("vendor:                %s\n", s->sc->about->vendor);
			printf("version:               %s\n", s->sc->about->version);
			printf("build:                 %s\n", s->sc->about->build);
			printf("localeVersion:         %s\n", s->sc->about->localeVersion);
			printf("localeBuild:           %s\n", s->sc->about->localeBuild);
			printf("osType:                %s\n", s->sc->about->osType);
			printf("productLineId:         %s\n", s->sc->about->productLineId);
			printf("apiType:               %s\n", s->sc->about->apiType);
			printf("apiVersion:            %s\n", s->sc->about->apiVersion);
			printf("instanceUuid:          %s\n", s->sc->about->instanceUuid);
			printf("licenseProductName:    %s\n", s->sc->about->licenseProductName);
			printf("licenseProductVersion: %s\n", s->sc->about->licenseProductVersion);
		}
		return 0;
	}

	/* if VC, use the vcuser account, host use sitescope account */
//	printf("product: name: %s, osType: %s, apiType: %s, apiVersion: %s\n",
//		s->sc->about->name, s->sc->about->osType, s->sc->about->apiType, s->sc->about->apiVersion);
	if (!user && strcmp(s->sc->about->apiType, "VirtualCenter") == 0) {
		if (!get_vcuser(server,port,vcuser))
			user = vcuser;
	}
	if (!user) user = "root";

	if (login_func(s, user, pass)) {
		printf("error logging in to server\n");
		return 1;
	}

	dprintf("do_sc: %d\n", do_sc);
	if (do_sc) return print_sc(s->sc);
	dprintf("do_perf: %d\n", do_perf);
	if (do_perf) return print_perf(s);

	/* Get a list of all host objects */
	if (have_obj) {
		dprintf("force_type: %d\n", force_type);
		gi_obj = &obj;
		if (force_type) {
			gi_type = type;
			gi_paths = (no_path ? 0 : paths);
		} else {
			gi_type = obj.type;
			gi_paths = paths;
		}
	} else {
		gi_type = type;
		gi_paths = paths;
		gi_obj = 0;
	}
	dprintf("gi_type: %p, gi_paths: %p, gi_obj: %p\n", gi_type, gi_paths, gi_obj);
	dprintf("gi_type: %s\n", (gi_type ? gi_type : "(null)"));
	dprintf("gi_paths[0]: %s\n", (gi_paths ? gi_paths[0] : "(null)"));
	if (gi_obj)
		dprintf("gi_obj: type: %s, value: %s\n", gi_obj->type, gi_obj->value);
	else
		dprintf("gi_obj: (null)\n");
	results = vim_getinfo(s, gi_type, gi_paths, (gi_paths ? 0 : 1), gi_obj);
	dprintf("results: %p\n", results);
	if (!results) {
		if (s->soap->error)
			soap_print_fault(s->soap, stdout);
		else
			printf("no results!\n");
		return 1;
	}

	i = list_count(results);
	dprintf("count: %d\n", i);
	level = 0;
	if (do_xml) tagstart("esxconf",level++);
	list_reset(results);
	while((ret = list_get_next(results)) != 0) {
		dprintf("obj: type: %s, name: %s\n", ret->obj.type, ret->obj.value);
		if (do_xml) {
			tagstart("ManagedObject",level++);
			if (!xmlcomp) for(i=0; i < level; i++) printf("  ");
			printf("<ref>%s:%s</ref>%s",ret->obj.type,ret->obj.value,(xmlcomp ? "" : "\n"));
			dump_xml(ret->nodes,level);
			tagend("ManagedObject",--level);
		} else {
			if (i > 1 || disp_mor) {
				if (do_vars) {
					printf("object=\'%s%c%s\'\n", ret->obj.type, OBJCHAR, ret->obj.value);
				} else {
					printf("object: %s%c%s\n", ret->obj.type, OBJCHAR, ret->obj.value);
				}
			}
			_disp(ret->nodes, 0);
		}
	}
	if (do_xml) tagend("esxconf",--level);
	vim_disconnect(s);
	return 0;

}
