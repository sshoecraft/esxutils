
#include "collect.h"
#include <stdarg.h>
#include "gendb_vm_disks.h"
#include <sys/stat.h>
#include <time.h>

#define DB_NAME "esxadmin"
#define DB_USER "esxadmin_ro"
#define DB_PASS "esxadmin_ro"

#define CIDB_NAME "sysdb"
#define CIDB_USER "sysdb_ro"
#define CIDB_PASS ""

#define DO_LOAD 0

int lprintf(FILE *fp, int level, char *fmt, ...) {
        char msg[1024], *p;
        va_list ap;
	int i;

	va_start(ap,fmt);
	p = msg;
	for(i=0; i < level; i++) {
		*p++ = ' ';
		*p++ = ' ';
	}
	vsprintf(p,fmt,ap);
	va_end(ap);
	return fprintf(fp,"%s", msg);
//	return printf("%s", msg);
}


#define BEGIN(name) lprintf(fp,l++,"<"#name">\n")
#define END(name) lprintf(fp,--l,"</"#name">\n")
#define STAG(ptr,field) lprintf(fp,l,"<"#field">%s</"#field">\n",ptr->field)
#define DTAG(ptr,field) lprintf(fp,l,"<"#field">%d</"#field">\n",ptr->field)
#define QTAG(ptr,field) lprintf(fp,l,"<"#field">%lld</"#field">\n",ptr->field)

void mksnap(struct csession *s) {
	struct farm *farm;
	struct host *host;
	struct datastore *ds;
	struct vm *vm;
	struct vm_disks_record *disk;
	char tmp[1024], ts[32], *home;
	struct tm *tptr;
	FILE *fp;
	int l;
	time_t t;

	/* Get HOME */
	home = getenv("HOME");
	if (!home) {
		printf("warning: no home var, not writing snap\n");
		return;
	}
	sprintf(tmp,"%s/archive",home);
	mkdir(tmp,0755);
	sprintf(tmp,"%s/archive/collect",home);
	mkdir(tmp,0755);
	if (chdir(tmp) < 0) {
		perror("chdir archive");
		return;
	}

	time(&t);
	tptr = gmtime(&t);
	sprintf(ts,"%04d%02d%02d%02d%02d%02d",1900+tptr->tm_year,tptr->tm_mon+1,tptr->tm_mday,
		tptr->tm_hour,tptr->tm_min,tptr->tm_sec);
	sprintf(tmp,"%s-%s.xml",s->vim->server,ts);
	fp = fopen(tmp,"w+");
	if (!fp) {
		perror("fopen archive");
		return;
	}

	l = 0;
	BEGIN(info);
	list_reset(s->farms);
	while((farm = list_next(s->farms)) != 0) {
		BEGIN(farm);
		STAG(farm,name);
		STAG(farm,object);
		list_reset(s->hosts);
		while((host = list_next(s->hosts)) != 0) {
			if (host->farm != farm) continue;
			BEGIN(host);
			STAG(host,name);
			STAG(host,object);
			STAG(host,uuid);
			STAG(host,os);
			STAG(host,version);
			DTAG(host,build);
			STAG(host,model);
			STAG(host,serial);
			STAG(host,bios);
			STAG(host,status);
			DTAG(host,cpu_pkgs);
			DTAG(host,cpu_count);
			DTAG(host,cpu_speed);
			DTAG(host,cpu_total);
//			DTAG(host,cons_mem);
			STAG(host,in_maint);
			STAG(host,subnet);
			list_reset(host->datastores);
			while((ds = list_next(host->datastores)) != 0) {
				BEGIN(datastore);
				STAG(ds,name);
				DTAG(ds,blocksize);
				DTAG(ds,total);
				DTAG(ds,free);
				END(datastore);
			}
			list_reset(s->vms);
			while((vm = list_next(s->vms)) != 0) {
				if (vm->host != host) continue;
				BEGIN(vm);
				STAG(vm,name);
				STAG(vm,object);
				STAG(vm,ci_name);
				STAG(vm,uuid);
				STAG(vm,os);
				STAG(vm,tools);
				STAG(vm,ip);
				STAG(vm,mac);
				DTAG(vm,cpu_total);
				DTAG(vm,mem_total);
				DTAG(vm,size_total);
				STAG(vm,state);
				STAG(vm,annotation);
				list_reset(vm->disks);
				while((disk = list_next(vm->disks)) != 0) {
					BEGIN(disk);
					STAG(disk,name);
					STAG(disk,filename);
					QTAG(disk,size);
					STAG(disk,isthin);
					END(disk);
				}
				END(vm);
			}
			END(host);
		}
		END(farm);
	}
	END(info);

	fclose(fp);

//	sprintf(tmp,"gzip %s-%s.xml",s->vim->server,ts);
//	system(tmp);

	return;
}

struct funcinfo {
	char *label;
	int (*func)(struct csession *s);
};
//#define PRIMITIVE_CAT(a, __VA_ARGS__) a ## _ ## __VA_ARGS__
#define FUNCDEF(A,F) { #A"_"#F, A ## _ ## F}
#define INITFUNC(NAME) FUNCDEF(init,NAME)
#define LOADFUNC(NAME) FUNCDEF(load,NAME)
#define GETFUNC(NAME) FUNCDEF(get,NAME)
#define UPDFUNC(NAME) FUNCDEF(update,NAME)
//#define GETFUNC(F) { "get_"#F, get_##F }
//#define UPDFUNC(F) { "update_"#F, update_##F }

static int callfuncs(struct csession *s, struct funcinfo *funcs) {
	register struct funcinfo *fip;

	for(fip = funcs; fip->label; fip++) {
		dprintf("fip: %p, fip->label: %p\n", fip, fip->label);
		dprintf("calling %s\n", fip->label);
		if (fip->func(s)) {
			dprintf("%s failed!\n", fip->label);
			return 1;
		}
		dprintf("back from %s\n", fip->label);
	}
	dprintf("done!\n");
	return 0;
}

int collect(char *req_server, int port, char *user, char *pass) {
	char *server;
	unsigned char *ap;
	struct vim_session *vim;
	struct csession *s;
	DB *db;
	int r;
	struct hostent *ent;
	struct funcinfo initfuncs[] = {
		INITFUNC(farms),
		INITFUNC(hosts),
		INITFUNC(vms),
		{ 0, 0 }
	};
#if DO_LOAD
	struct funcinfo loadfuncs[] = {
		LOADFUNC(farms),
		LOADFUNC(hosts),
		LOADFUNC(vms),
		LOADFUNC(datastores),
		LOADFUNC(host_datastores),
		{ 0, 0 }
	};
#endif
	struct funcinfo getfuncs[] = {
		GETFUNC(farms),
		GETFUNC(datastores),
		GETFUNC(hosts),
		GETFUNC(vms),
		{ 0, 0 }
	};
	struct funcinfo updfuncs[] = {
		UPDFUNC(farms),
		UPDFUNC(hosts),
		UPDFUNC(vms),
		UPDFUNC(datastores),
		UPDFUNC(host_datastores),
		{ 0, 0 }
	};

	ent = getdnshostent(req_server);
	dprintf("ent: %p\n", ent);
	if (!ent) {
		printf("error: unable to find server in dns: %s\n", req_server);
		return 1;
	}
	dprintf("h_name: %s\n", ent->h_name);
	server = 0;
	if (strncmp(ent->h_name,req_server,strlen(req_server)) != 0) {
		char **p;

		for(p=ent->h_aliases; *p; p++) {
			dprintf("alais: %s\n", *p);
			if (strncmp(*p,req_server,strlen(req_server)) == 0) {
				server = *p;
				break;
			}
		}
	}
	if (!server) server = ent->h_name;

	dprintf("server: %s, port: %d, user: %s, pass: %s\n", server, port, user, pass);
	dprintf("user: %p, pass: %p\n", user, pass);

	s = 0;
	db = 0;
	r = 1;

	dprintf("connecting...\n");
	vim = vim_connect(server, port);
	if (!vim) {
		printf("vim_connect failed for server: %s\n", server);
		return 1;
	}

        /* if VC, get the account name vcservers file, host use sitescope account */
	dprintf("product: name: %s, osType: %s, apiType: %s, apiVersion: %s\n",vim->sc->about->name, vim->sc->about->osType, vim->sc->about->apiType, vim->sc->about->apiVersion);
	dprintf("user: %s\n", user);
	if (!user[0]) {
		if (strcmp(vim->sc->about->apiType, "VirtualCenter") == 0) {
			if (get_vcuser(server,port,user)) {
				printf("warning: unable to get user for vcserver: %s:%d, using root!\n",server,port);
				strcpy(user,"root");
			}
		}
	}
	dprintf("pass: %s\n", pass);

	if (vim_login(vim, user, pass)) {
		printf("collect: error logging in to %s\n",server);
		return 1;
	}

	dprintf("Connecting to db...\n");
	db = malloc(sizeof(*db));
	if (db_connect(db,DB_NAME,DB_USER,DB_PASS)) {
		printf("collect: unable to connect to db!\n");
		goto done;
	}

	s = malloc(sizeof(*s));
	s->vim = vim;
	s->db = db;
	s->farms = list_create();
	s->datastores = list_create();
	s->hosts = list_create();
	s->vms = list_create();
	ap = (unsigned char *) ent->h_addr;
	sprintf(s->addr,"%d.%d.%d.%d",ap[0],ap[1],ap[2],ap[3]);
	dprintf("addr: %s\n", s->addr);

	s->cidb = malloc(sizeof(*db));
	if (db_connect(s->cidb,CIDB_NAME,CIDB_USER,CIDB_PASS)) {
		printf("collect: unable to connect to cidb!\n");
		goto done;
	}

	/* XXX order is important here */

	if (callfuncs(s,initfuncs)) goto done;

	dprintf("back from initfuncs\n");

#if DO_LOAD
	/* XXX can't use this until you put objects into db */
	if (callfuncs(s,loadfuncs)) goto done;
#endif

	/* Collect from VMware */
	if (callfuncs(s,getfuncs)) goto done;

	dprintf("do_update: %d", do_update);
	if (do_update) {
		/* fixup lists & update db */
		if (callfuncs(s,updfuncs)) goto done;

		/* Write snapshot */
//		printf("do_mksnap: %d\n", do_mksnap);
		if (do_mksnap) mksnap(s);
	}

	r = 0;

        printf("farms: %d\n", list_count(s->farms));
        printf("datastores: %d\n", list_count(s->datastores));
        printf("hosts: %d\n", list_count(s->hosts));
        printf("vms: %d\n", list_count(s->vms));

done:
	if (db) db_disconnect(db);
	if (vim) vim_disconnect(vim);
	return r;
}
