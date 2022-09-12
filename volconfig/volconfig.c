
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include "db.h"
#include "util.h"
#include "list.h"

#define DB_NAME "mbs"
#define DB_USER "mbs_ro"
#define DB_PASS "mbs2011readonly**"

#define BUFFER_SIZE 4*1048576

#ifdef dprintf
#undef dprintf
#endif

int debug;
#define dprintf(level, format, args...) { if (debug >= level) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args); }

struct datadesc {
	int num;
	int type;
	void *dest;
};

enum DD_TYPES { DD_UNK, DD_INT, DD_STR };

void getdesc(struct datadesc *desc, char *line) {
	char temp[256], *p;

	while(desc->dest) {
		dprintf(2,"desc: num: %d, type: %d, dest: %p\n", desc->num, desc->type, desc->dest);
		strcpy(temp,strele(desc->num, ",", line));
		dprintf(2,"temp: %s\n", temp);
		p = strele(1, "=", temp);
		dprintf(2,"p: %s\n", p);
		switch(desc->type) {
		case DD_INT:
			*((int *)desc->dest) = atoi(p);
			break;
		case DD_STR:
			strcpy((char *)desc->dest,p);
			break;
		}
		desc++;
	}
}

void getszqty(char *size, char *qty, char *line) {
	char *p;

	// 20GB x 1
	dprintf(3,"line: %s\n", line);
	strcpy(size,strele(0," x ",line));
	dprintf(3,"size: %s\n", size);
	p = strstr(size,"GB");
	if (p) {
		*p = 0;
		dprintf(3,"NEW size: %s\n", size);
	}
	strcpy(qty,strele(1," x ",line));
	dprintf(3,"qty: %s\n", qty);
}

struct vginfo {
	char name[32];
	char size[8];
	char qty[8];
	char total[8];
	char tier[8];
};

void get_vginfo(struct vginfo *info, char *line) {
	char temp[32];
	// Size=20GB x 1, TOTAL=20, StTier=1, VG=vg04
	struct datadesc vgdesc[] = {
		{ 0, DD_STR, temp },
		{ 1, DD_STR, &info->total },
		{ 2, DD_STR, &info->tier },
		{ 3, DD_STR, &info->name },
		{ 0, 0, 0 }
	};

	memset(info,0,sizeof(*info));
	getdesc(vgdesc,line);

	/* temp has both size and qty */
	getszqty(info->size, info->qty, temp);

	dprintf(1,"vginfo: name: %s, size: %s, qty: %s, total: %s, tier: %s\n", info->name, info->size, info->qty, info->total, info->tier);
}

struct userinfo {
	char name[64];
	char groupno[8];
	char sea[64];
	char shell[64];
	char purpose[256];
};

void get_userinfo(struct userinfo *info, char *line) {
	// User=root, GroupNo=, SEA=, Shell=, Purpose=
	struct datadesc userdesc[] = {
		{ 0, DD_STR, &info->name },
		{ 1, DD_STR, &info->groupno },
		{ 2, DD_STR, &info->sea },
		{ 3, DD_STR, &info->shell },
		{ 4, DD_STR, &info->purpose },
		{ 0, 0, 0 }
	};

	memset(info,0,sizeof(*info));
	getdesc(userdesc,line);
	dprintf(1,"userinfo: name: %s, groupno: %s, sea: %s, shell: %s, purpose: %s\n", info->name, info->groupno, info->sea, info->shell, info->purpose);
}

struct driveinfo {
	char path[256];
	char type[32];
	char size[8];
	char qty[8];
	char group[64];
	char groupno[8];
	char owner[64];
	char perm[8];
	char total[8];
	char tier[8];
	char vg[64];
};

void get_driveinfo(struct driveinfo *info, char *line) {
	char temp[256];
	// /opt/fast/esp  Type=dedicated, Size=GB x , Grp=fastgrp, GroupNo=, Own=fastuser, Perm=755, TOTAL=10, StTier=, VG=vg01
	struct datadesc drivedesc[] = {
		{ 0, DD_STR, &info->type },
		{ 1, DD_STR, temp },
		{ 2, DD_STR, &info->group },
		{ 3, DD_STR, &info->groupno },
		{ 4, DD_STR, &info->owner },
		{ 5, DD_STR, &info->perm },
		{ 6, DD_STR, &info->total },
		{ 7, DD_STR, &info->tier },
		{ 8, DD_STR, &info->vg },
		{ 0, 0, 0 }
	};

	memset(info,0,sizeof(*info));
	getdesc(drivedesc,line);

	/* XXX Special for path */
	strcpy(info->path,strele(0," ",line));

	/* temp has size and qty */
	getszqty(info->size, info->qty, temp);

	dprintf(1,"driveinfo: path: %s, type: %s, size: %s, qty: %s, group: %s, groupno: %s, owner: %s, perm: %s, total: %s, tier: %s, vg: %s\n", info->path, info->type, info->size, info->qty, info->group, info->groupno, info->owner, info->perm, info->total, info->tier, info->vg);
}

struct volconfig {
	list drives;
	list vgs;
	list users;
};

void parse(struct volconfig *config, char *line) {
	struct driveinfo newdrv;
	struct vginfo newvg;
	struct userinfo newuser;

	trim(line);
	if (!strlen(line)) return;
	dprintf(2,"line: %s\n", line);

	if (strncmp(line,"Size=",5) == 0) {
		get_vginfo(&newvg,line);
		list_add(config->vgs, &newvg, sizeof(newvg));

	} else if (strncmp(line,"User=",5) == 0) {
		get_userinfo(&newuser,line);
		list_add(config->users, &newuser, sizeof(newuser));

	} else if (strstr(line,"Type=")) {
		get_driveinfo(&newdrv,line);
		list_add(config->drives, &newdrv, sizeof(newdrv));
	}
}

int usage(char *prog, int r) {
	printf("usage: %s [options] <ci>\n", prog);
	printf("   where options are:\n");
	printf("    -d            show runtime information\n");
	printf("    -r            dump raw text and exit\n");
	exit(r);
}

int main(int argc, char **argv) {
	char query[128], *host, *text;
	char line[1024], *p;
	DB db;
	int found,i;
	struct volconfig config;
	struct driveinfo *drive;
	struct vginfo *vg;
	struct userinfo *user;
	int ch,raw;

	debug = raw = 0;
	host = 0;
	while((ch = getopt(argc, argv, "dr")) != -1) {
		switch(ch) {
		case 'd':
			debug++;
			break;
		case 'r':
			raw=1;
			break;
		}
	}
	dprintf(1,"debug: %d\n", debug);

	dprintf(1,"optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(argv[0],1);
	host = argv[optind];
	dprintf(1,"host: %s\n", host);

	text = malloc(BUFFER_SIZE);
	if (!text) {
		perror("malloc");
		return 1;
	}

	if (db_connect(&db,DB_NAME,DB_USER,DB_PASS)) {
		printf("error: unable to connect to db!\n");
		return 1;
	}
	found = 0;
	sprintf(query,"SELECT \"Volume Config\" FROM MBS WHERE hostname = '%s'",host);
	if (db_exec(&db,query) == 0) {
		if (db_fetch(&db) == 0) {
			SQLGetData(db.hstmt, 1, SQL_C_CHAR, (SQLPOINTER) text, BUFFER_SIZE, 0);
			dprintf(1,"text:\n%s\n", text);
			found = 1;
		}
		db_fetch_done(&db);
	}
	db_disconnect(&db);
	if (!found) {
		printf("error: unable to get config!\n");
		return 1;
	}
	if (raw) {
		if (!debug) printf("%s\n", text);
		return 0;
	}

	config.drives = list_create();
	config.vgs = list_create();
	config.users = list_create();

	i = 0;
	for(p = text; *p; p++) {
//		printf("p: %x, lastp: %x\n",*p,lastp);
		if (*p == '\n' || *p == '\r') {
			line[i] = 0;
			parse(&config,line);
			i = 0;
		} else {
			line[i++] = *p;
		}
	}
	if (i) {
		line[i] = 0;
		parse(&config,line);
	}

	if (!list_count(config.drives)) {
		printf("error: no volumes parsed!\n");
		return 1;
	}

	printf("<?xml version=\"1.0\" encoding=\"utf-8\" ?>\n");
	printf("<DriveLayout>\n");
	list_reset(config.drives);
	while((drive = list_get_next(config.drives)) != 0) {
		printf("  <Drive driveVolume=\"%s\" sanType=\"%s\" lunSize=\"%s\" lunQty=\"%s\" Group=\"%s\" Owner=\"%s\" Permissions=\"%s\" totalSpace=\"%s\" VolumeGroup=\"%s\" />\n", drive->path, drive->type, drive->size, drive->qty, drive->group, drive->owner, drive->perm, drive->total, drive->vg);
	}
	if (list_count(config.vgs)) {
		printf("  <VolumeGroups>\n");
		list_reset(config.vgs);
		while((vg = list_get_next(config.vgs)) != 0) {
			printf("    <VolumeGroup lunSize=\"%s\" lunQty=\"%s\" totalSpace=\"%s\" VolumeGroup=\"%s\" storageTier=\"%s\" />\n",vg->size, vg->qty, vg->total, vg->name, vg->tier);
		}
		printf("  </VolumeGroups>\n");
	}
	if (list_count(config.users)) {
		printf("  <VolumeUserInfo>\n");
		list_reset(config.users);
		while((user = list_get_next(config.users)) != 0) {
			printf("    <VolumeUser user=\"%s\" groupNo=\"%s\" sea=\"%s\" shell=\"%s\" purpose=\"%s\" />\n",user->name, user->groupno, user->sea, user->shell, user->purpose);
		}
		printf("  </VolumeUserInfo>\n");
	}
	printf("</DriveLayout>\n");
	return 0;
}
