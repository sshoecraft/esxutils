
#include "esxpost.h"
#include "soapfuncs/ServiceContent.h"
#include "soapfuncs/RetrieveProperties.h"
#include "soapfuncs/ClusterConfigSpec.h"
#include "soapfuncs/OptionValue.h"
#include "soapfuncs/Destroy.h"

//extern int skip_mtp;
int makevols = 0;

void add_opt(list opts, char *name, char *val) {
	struct OptionValue *op,opt;
	int found;

	dprintf("name: %s, val: %s\n", name, val);
	found = 0;
	list_reset(opts);
	while((op = list_get_next(opts)) != 0) {
		if (strcmp(op->key,name) == 0) {
			found = 1;
			break;
		}
	}
	dprintf("found: %d\n", found);
	if (!found) {
		opt.key = calloc(strlen(name)+1,1);
		strcat(opt.key,name);
		opt.value = calloc(strlen(val)+1,1);
		strcat(opt.value,val);
		list_add(opts,&opt,sizeof(opt));
	}
	dprintf("added...\n");
}

int read_options(struct vim_session *vim,char *filename,list opts) {
	FILE *fp;
	char line[128],options[256],vexp[16],name[128];

	sprintf(options,"%s/%s",etcdir,filename);
	dprintf("options: %s\n", options);
	fp = fopen(options,"r");
	if (!fp) {
		/* XXX Not an error */
		return 0;
	}
	while(fgets(line,sizeof(line),fp)) {
		trim(line);
		dprintf("line: %s\n", line);
		if (line[0] == '#') continue;
		// line format: Site   Cluster   Datacenter
		strcpy(vexp,strele(0," ",line));
		trim(vexp);
		dprintf("version: %s, vexp: %s\n", vim->sc->about->version, vexp);
		if (match(vim->sc->about->version,vexp)) {
			strcpy(name,strele(1," ",line));
			trim(name);
			printf("adding option: name: %s\n", name);
			add_opt(opts, name, trim(strele(2," ",line)));
		}
	}
	fclose(fp);

	return 0;
}

static struct ManagedObjectReference *gethostFolder(struct vim_session *vim, struct ManagedObjectReference *dc) {
	char *paths[] = { "hostFolder", 0 };
	int count;
	list res;
	struct returnval *ret;
	struct propSet *set;
	struct ManagedObjectReference *folder;

	res = vim_getinfo(vim,dc->type,paths,0,dc);
	count = list_count(res);
	dprintf("count: %d\n", count);
	if (!count) {
		printf("create cluster: unable to get hostFolder from dc: %s:%s\n", dc->type, dc->value);
		return 0;
	}
	ret = res->first->item;
	set = get_result(ret->nodes,"hostFolder");
		if (!set) {
		printf("gethostFolder: unable to find hostFolder in results!\n");
		return 0;
	}

	folder = malloc(sizeof(struct ManagedObjectReference *));
	folder->type = set->type;
	folder->value= set->value;
	return folder;
}

struct ManagedObjectReference *create_cluster(struct vim_session *vim, struct ManagedObjectReference *dc, char *name) {
	struct ClusterDasConfigInfo das;
	struct ClusterDrsConfigInfo drs;
	struct ClusterConfigSpec config;
	struct ManagedObjectReference *folder;
	struct ClusterDasVmSettings dasVm;

	memset(&dasVm,0,sizeof(dasVm));
//	dasVm.restartPriority = "medium";
	dasVm.isolationResponse = "powerOff";

	memset(&das,0,sizeof(das));
	das.failoverLevel = "1";
	das.admissionControlEnabled = 0;
	das.defaultVmSettings = &dasVm;
	das.enabled = 0;
	das.option = list_create();
	read_options(vim,"dasoptions",das.option);

	memset(&drs,0,sizeof(drs));
	drs.enabled = 0;
	drs.option = list_create();
	read_options(vim,"drsoptions",drs.option);

	memset(&config,0,sizeof(config));
	config.dasConfig = &das;
	config.drsConfig = &drs;

	folder = gethostFolder(vim,dc);
	if (!folder) return 0;

	return vim_createcluster(vim,folder,name,&config);
}

static struct ManagedObjectReference *get_cluster(struct esxpost *info) {
	struct ManagedObjectReference *dc_obj,*cl_obj;
	char cluster[64];
	int tries;

	/* Find the DC */
	dprintf("finding dc...\n");
	dc_obj = vim_findobj(info->vim, "Datacenter", info->dc);
	if (!dc_obj) {
		dprintf("creating dc...\n");
		dc_obj = vim_createdc(info->vim, info->dc);
		if (!dc_obj) {
			printf("error: unable to create dc: %s\n", info->dc);
			return 0;
		}
	}
	dprintf("dc: %s:%s\n", dc_obj->type, dc_obj->value);

	tries = 3;
cl_again:
	/* Find the cluster */
	dprintf("finding cluster...\n");
//	strcpyuc(cluster,info->cluster,sizeof(cluster));
	strcpy(cluster,info->cluster);
	cl_obj = vim_findobj(info->vim, "ClusterComputeResource", cluster);
	if (!cl_obj) {
		dprintf("creating cluster: %s\n",cluster);
		cl_obj = create_cluster(info->vim, dc_obj, cluster);
		if (!cl_obj) {
			if (tries--) {
				sleep(5);
				goto cl_again;
			}
			printf("error: unable to create cluster: %s\n", cluster);
			return 0;
		}
		makevols++;
	}

	return cl_obj;

}

int joinvc(struct esxpost *info) {
	struct ManagedObjectReference *host_obj;
	int iscon;

	/* Find the host */
	dprintf("finding host...\n");
	host_obj = vim_findobj(info->vim, "HostSystem", info->fqdn);
	if (host_obj) {
		/* If the host is disconnected */
		dprintf("checking conx status...\n");
		iscon = vim_isconnected(info->vim, host_obj);
		if (iscon && rejoin) {
			vim_disconnect_host(info->vim, host_obj, 0);
			sleep(30);
			iscon = 0;
		}
		if (!iscon) {
			dprintf("keylen: %d\n", (int)strlen(info->key));
			if (strlen(info->key)) {
				/* remove & re-add */
				if (vim_destroy(info->vim,host_obj)) {
					printf("error unable to remove host!\n");
					return 1;
				}
				host_obj = 0;
				makevols--;
			} else if (con_host(info,host_obj) != 0) {
				printf("error: unable to reconnect host!\n");
				return 1;
			}
		}
	}
	if (!host_obj) {
		struct ManagedObjectReference *cl_obj;

		cl_obj = get_cluster(info);
		if (!cl_obj) return 1;

		host_obj = add_host(info,cl_obj);
		if (!host_obj) return 1;
		makevols++;
	}

	/* If not in maint mode, enter mm (unless no specified) */
	dprintf("checking if in maint mode...\n");
	if (vim_ismm(info->vim, host_obj) == 0 && nomaint == 0) {
		dprintf("entering mm...\n");
		if (vim_entermm(info->vim, host_obj,0))
			return 1;
	}

	return 0;
}

int post(struct esxpost *info) {
	makevols = 0;
	joinvc(info);
	/* We must both add the cluster AND add the host for makevols to be emitted */
	dprintf("makevols: %d\n", makevols);
	if (makevols == 2) printf("+++MAKEVOLS+++\n");

	dprintf("done!\n");
	return 0;
}
