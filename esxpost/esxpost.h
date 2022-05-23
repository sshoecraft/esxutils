
#ifndef __ESXPOST_H
#define __ESXPOST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "vim.h"
#include "util.h"
#include "encrypt.h"
#include "encode.h"

struct esxpost {
	char fqdn[256];
	char addr[32];
	char subnet[32];
	char gateway[32];
	char gen[16];
	char name[128];
	char domain[128];
	char key[128];
	char version[16];
	int site;
	int rcs;
	int ng2;
	char vcserver[256];
	int vcport;
	char vcuser[64];
	char dc[32];
	char cluster[128];
	char license[256];
	struct ManagedObjectReference *host_obj;
	struct vim_session *vim;
	char db_name[32];
	char db_user[32];
	char db_pass[32];
	char mbs_name[32];
	char mbs_user[32];
	char mbs_pass[32];
	char mbs_query[256];
};

struct destinfo {
	char str[64];
	int idx;
};

int get_info(struct esxpost *,char *);
char *strcpylc(char *,char *,int);
char *strcpyuc(char *,char *,int);
int post(struct esxpost *);
int clear(struct esxpost *);
struct ManagedObjectReference *add_host(struct esxpost *, struct ManagedObjectReference *);
int con_host(struct esxpost *, struct ManagedObjectReference *);

#ifdef __MINGW32__
#define sleep(x) Sleep(x)
#endif

/* Globals */
extern char etcdir[256], *farm, *hostip;
extern int no,clear_alarms,real,nomaint,rejoin,curf,dorem;

#endif
