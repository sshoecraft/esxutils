
#include "esxpost.h"
#ifdef __MINGW32__
#include "getopt.h"
#endif
#include "cfg.h"
#include "conv.h"

#include "version.h"

int no,clear_alarms,real,nomaint,rejoin,curf,dorem;
char *farm;
//int skip_mtp;
char etcdir[256];
char *hostip;

void usage(int r) {
	printf("usage: esxpost [options]\n");
	printf("  where options are:\n");
	printf("    -a             do not override host info with config files (cluster/etc)\n");
	printf("    -c             clear alarms\n");
	printf("    -n             don't check connection status when clearing alarms\n");
	printf("    -m             don't put host in maint mode when adding/reconnecting\n");
	printf("    -j             join even if already connected\n");
	printf("    -i             specify IP\n");
//	printf("    -s             skip mtp (for 4.x servers)\n");
	printf("    -g             dump info for genconfig\n");
	printf("    -v             specify ESX[i] host version\n");
	printf("    -k             specify sslThumprint\n");
	printf("    -f             force farm name\n");
	printf("    -r             remove instead of disconnect/reconnect\n");
	printf("    -e             use current farm\n");
	printf("    -V             display my version\n");
	exit(r);
}

int get_config(struct esxpost *info, char *filename) {
	CFG_INFO *cfg;
	struct cfg_proctab tab[] = {
		{ "esxadmin", "datasource", "", DATA_TYPE_STRING, &info->db_name, sizeof(info->db_name), "" },
		{ "esxadmin", "username", "", DATA_TYPE_STRING, &info->db_user, sizeof(info->db_user), "" },
		{ "esxadmin", "password", "", DATA_TYPE_STRING, &info->db_pass, sizeof(info->db_pass), "" },
		{ "mbs", "datasource", "", DATA_TYPE_STRING, &info->mbs_name, sizeof(info->mbs_name), "" },
		{ "mbs", "username", "", DATA_TYPE_STRING, &info->mbs_user, sizeof(info->mbs_user), "" },
		{ "mbs", "password", "", DATA_TYPE_STRING, &info->mbs_pass, sizeof(info->mbs_pass), "" },
		{ "mbs", "query", "", DATA_TYPE_STRING, &info->mbs_query, sizeof(info->mbs_query), "" },
		{ 0,0,0,0,0,0 }
	};

	dprintf("filename: %s\n", filename);
	cfg = cfg_read(filename);
	if (!cfg) {
		perror("cfg_read");
		return 1;
	}
	cfg_get_tab(cfg, tab);
//	cfg_disp_tab(tab,"esxadmin",0);
//	dprintf("db_name: %s, db_user: %s, db_pass: %s\n", info->db_name, info->db_user, info->db_pass);
	return 0;
}


int main(int argc, char **argv) {
	char *host,*key,*vers;
	int ch,r,dump_gen;
	struct esxpost info;
	char filename[256];

	/* Don't buffer stdout */
	setvbuf(stdout,NULL,_IONBF,0);

	key = vers = farm = hostip = 0;
	r = clear_alarms = no = dump_gen = real = nomaint = rejoin = curf = dorem = 0;
//	skip_mtp = 0;
	while((ch = getopt(argc, argv, "cnmejVhsgrk:v:f:i:")) != -1) {
		switch(ch) {
		case 'a':
			real = 1;
			break;
		case 'c':
			clear_alarms = real = 1;
			break;
		case 'n':
			no = 1;
			break;
		case 'm':
			nomaint = 1;
			break;
		case 'j':
			rejoin = 1;
			break;
#if 0
		case 's':
			skip_mtp = 1;
			break;
#endif
		case 'g':
			dump_gen = 1;
			break;
		case 'i':
			hostip = optarg;
			break;
		case 'k':
			key = optarg;
			break;
		case 'f':
			farm = optarg;
			break;
		case 'e':
			curf = 1;
			break;
		case 'r':
			dorem = 1;
			break;
		case 'v':
			vers = optarg;
			break;
		case 'V':
			printf("esxpost version %s\n", VERSIONSTR);
			printf("by Steve Shoecraft (sshoecraft@earthlink.net)\n");
			return 0;
			break;
		case 'h':
			usage(0);
			break;
		case '?':
			usage(1);
			break;
		}
	}

	dprintf("optind: %d, argc: %d\n", optind, argc);
	if (optind >= argc) usage(1);
	host = argv[optind++];
	dprintf("host: %s\n", host);

	strcpy(etcdir,ETCDIR);
	dprintf("etcdir: %s\n", etcdir);

	/* Read config */
	memset(&info,0,sizeof(info));
	sprintf(filename,"%s/esxpost.conf",etcdir);
	if (get_config(&info,filename)) return 1;

	if (key) strcpy(info.key,key);
	if (vers) strcpy(info.version,vers);

	r = get_info(&info,host);
	if (r) goto done;

	/* Dump info to genconf */
	dprintf("version: %s\n", info.version);
	if (dump_gen) {
		/* vcserver,dc,cluster,license,addr,subnet,gw,gen */
		printf("INFO: %s,%s,%s,%s,%s,%s,%s,%s\n",info.vcserver,info.dc,info.cluster,info.license,info.addr,info.subnet,info.gateway,info.gen);
		return 0;
	}

	/* Change to /tmp so our logs get saved */
	if (strcmp(etcdir,".") != 0) chdir("/tmp");

	dprintf("connecting...\n");
	if ((info.vim = vim_connect(info.vcserver,info.vcport)) == 0) {
		printf("error: unable to connect to vcserver: %s\n", info.vcserver);
		goto done;
	}
	dprintf("logging in...\n");
	dprintf("info.vcuser: %s\n", info.vcuser);
	if (!strlen(info.vcuser)) {
		if (get_vcuser(info.vcserver,info.vcport,info.vcuser)) {
			printf("error: unable to get vcuser for server: %s\n", info.vcserver);
			goto done;
		}
	}
	if (vim_login(info.vim,info.vcuser,0)) {
		printf("error: unable to login to vcserver: %s\n", info.vcserver);
		goto done;
	}

	r = 1;
	if (clear_alarms)
		r = clear(&info);
	else
		r = post(&info);

done:
	dprintf("returning: %d\n", r);
	return r;
}
