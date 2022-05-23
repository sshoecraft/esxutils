
#ifndef __ESXPERF_H
#define __ESXPERF_H

#include "vim.h"
#include "util.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#if 0
struct config {
	char db_name[32];
	char db_user[32];
	char db_pass[32];
	char table_name[32];
	char time_field[16];
	char hosts_table;
	int use_host_id;
	char host_field[16];
	char cpu_field[16];
	char mem_field[16];
	char disk_field[16];
	char net_field[16];
};
#endif

struct esxperf {
	char timestamp[32];
	int cpu;
	int mem;
	int disk;
	int net;
};

extern int verbose;
//extern int get_config(struct config *, char *);

#define vprintf(format, args...) if (verbose) printf("%s: " format,__FUNCTION__,## args)

#endif
