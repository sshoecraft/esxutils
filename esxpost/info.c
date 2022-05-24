
/* Determine which VC server */
/* Determine which Datacenter */
/* Determine which farm (MBS) */
/* Determine which network (callsub) */

#include "esxpost.h"
#include "socket.h"
#include "gendb_hosts.h"
#include "gendb_farms.h"
#include "util.h"
#include "soapfuncs/ServiceContent.h"

#ifdef __MINGW32__
#include "windows.h"
#include "winsock2.h"
#else
#include <netinet/in.h>
#include <netdb.h>
#endif
#include <ctype.h>

#define CALLSUB_CMD "/usr/local/bin/callsub"

static struct hostent *_gethost(char *name) {
	struct hostent *he;
	int retries,r;

	dprintf("name: %s\n", name);

	retries = 3;
again:
	r = 0;
        he = gethostbyname(name);
	if (!he) {
		switch(h_errno) {
		case HOST_NOT_FOUND:
			dprintf("HOST_NOT_FOUND\n");
			break;
		case NO_DATA:
			dprintf("NO_DATA\n");
			break;
		case NO_RECOVERY:
			dprintf("NO_RECOVERY\n");
			break;
		case TRY_AGAIN:
			if (retries--) goto again;
			break;
		}
		return 0;
        }
	return he;
}

static int callsub(struct esxpost *info) {
#if 0
	char line[128],tmpfile[128];
	int status,lines,have_site;
	FILE *fp;

	sprintf(tmpfile,"/dev/shm/callsub_out_%d",getpid());
	sprintf(line,"%s -n -d %s > %s 2>/dev/null", CALLSUB_CMD, info->fqdn, tmpfile);
	dprintf("cmd: %s\n", line);
	status = system(line);
#if 0
	if (status >= 256) s >>= 8;
	dprintf("status: %d\n", status);
	if (status != 0) {
		printf("error: error getting callsub info (status: %d)!\n", status);
		return 1;
	}
#endif

	fp = fopen(tmpfile,"r");
	if (!fp) {
		printf("error: unable to open %s!\n",tmpfile);
		unlink(tmpfile);
		return 1;
	}

	/* Read the response */
	lines = have_site = 0;
	dprintf("getting response...\n");
	while(fgets(line,sizeof(line),fp) != 0) {
		lines++;
		dprintf("line: %s",line);
#define IP "IP:"
		if (strncmp(line,IP,sizeof(IP)-1) == 0) {
			char temp[16];

			temp[0] = 0;
			strncat(temp,trim(strele(1,":",line)),sizeof(temp)-1);
			dprintf("ip: %s, temp: %s\n", info->addr, temp);
			if (strcmp(info->addr,temp) != 0) {
				printf("error: callsub returned a different address than dns (%s != %s)!\n", temp, info->addr);
				unlink(tmpfile);
				return 1;
			}
#define SUBNET "Subnet:"
		} else if (strncmp(line,SUBNET,sizeof(SUBNET)-1) == 0) {
			strncat(info->subnet,trim(strele(1,":",line)),sizeof(info->subnet)-1);
			dprintf("subnet: %s\n", info->subnet);
#define GATEWAY "Gateway:"
		} else if (strncmp(line,GATEWAY,sizeof(GATEWAY)-1) == 0) {
			strncat(info->gateway,trim(strele(1,":",line)),sizeof(info->gateway)-1);
			dprintf("gateway: %s\n", info->gateway);
#define SITE "SITE:"
		} else if (strncmp(line,SITE,sizeof(SITE)-1) == 0) {
			strcpy(info->gen,(strstr(line,"NG2") ? "NG2" : "NG1"));
			have_site = 1;
			break;
		}
	}
	unlink(tmpfile);
	if (!have_site) {
		printf("error: callsub did not return SITE data, omnia update needed?\n");
		return 1;
	}
	if (!lines) {
		printf("error: callsub returned no data!\n");
		return 1;
	}

	fclose(fp);
#else
	char temp[32],*gw,*p;
	gw = ".1";
	strcpy(temp,info->addr);
	dprintf("temp: %s\n", temp);
	p = strrchr(temp,'.');
	if (!p) {
		printf("WARNING: info->addr has no '.' (%s)\n", info->addr);
		strcpy(temp,"0.0.0");
	} else {
		*p = 0;
	}
	dprintf("temp: %s\n", temp);
	strcpy(info->subnet,temp);
	strcat(info->subnet,".0");
	strcpy(info->gateway,temp);
	strcat(info->gateway,gw);
	strcpy(info->gen,"NG1");
#endif
	return 0;
}

static int get_version(struct esxpost *info) {
	struct vim_session *s;

	dprintf("getting version...\n");
	s = vim_connect(info->fqdn,0);
	if (!s) {
		printf("error: unable to connect to the host!\n");
		return 1;
	}
	strncat(info->version,s->sc->about->version,sizeof(info->version));
	vim_disconnect(s);
	dprintf("version: %s\n", info->version);
	return 0;
}

static int get_vc(struct esxpost *info) {
	if (get_vcserver(info->version,info->cluster,info->fqdn,info->vcserver,info->vcport,info->vcuser)) {
		strcpy(info->vcserver,"unknown");
		strcpy(info->vcuser,"unknown");
	}
	return 0;
}

static int get_dc(struct esxpost *info) {
	FILE *fp;
	char line[1024],datacenters[256],sitestr[8];
	char vexp[32],sexp[16],cexp[128],hexp[128],fmt[128];
	int field,found;

	dprintf("site: %d\n", info->site);
	sprintf(datacenters,"%s/datacenters", etcdir);
	dprintf("datacenters: %s\n", datacenters);
	fp = fopen(datacenters,"r");
	if (!fp) {
		printf("error: unable to read datacenters file (%s)!\n", datacenters);
		return 1;
	}
	found = 0;
	while(fgets(line,sizeof(line),fp)) {
		strcpy(line,stredit(line,"TRIM,COMPRESS"));
		dprintf("line: %s\n", line);
		if (line[0] == '#') continue;
		// #VC Server,Site,Cluster,Format
		field = 0;
		strcpy(vexp,strele(field++,",",line));
		dprintf("vcserver: %s, vexp: %s\n", info->vcserver, vexp);
		if (!match(info->vcserver,vexp)) continue;
		dprintf("vcserver matched.\n");
		sprintf(sitestr,"%d",info->site);
		strcpy(sexp,strele(field++,",",line));
		dprintf("sitestr: %s, sexp: %s\n", sitestr, sexp);
		if (!match(sitestr,sexp)) continue;
		dprintf("site matched\n");
//		strcpyuc(cexp,strele(field++,",",line),sizeof(cexp));
		strcpy(cexp,strele(field++,",",line));
		dprintf("cluster: %s, cexp: %s\n", info->cluster, cexp);
		if (!match(info->cluster,cexp)) continue;
		dprintf("cluster matched\n");
		strncpy(hexp,strele(field++,",",line),sizeof(hexp));
		dprintf("host: %s, hexp: %s\n", info->fqdn, hexp);
		if (!match(info->fqdn,hexp)) continue;
		dprintf("host matched\n");
		strcpy(fmt,strele(field++,",",line));
		dprintf("fmt: %s\n", fmt);
		sprintf(info->dc,fmt,info->site);
		dprintf("dc: %s\n", info->dc);
		found = 1;
		break;
	}
	fclose(fp);

	dprintf("found: %d\n", found);
	return (found == 0);
}

static char current_farm[256];

static int get_cluster(struct esxpost *info) {
	DB db;
	char query[256],*p;

	dprintf("curf: %d\n", curf);
	if (curf) {
		strcpy(info->cluster,current_farm);
	} else {
	/* Connect to MBS db */
	if (strlen(info->mbs_name)) {
		dprintf("connecting to MBS...\n");
		if (db_connect(&db,info->mbs_name,info->mbs_user,info->mbs_pass)) return 1;

		/* XXX MBS Hostname is NON-FQDN - LOL */
		sprintf(query,info->mbs_query,info->name);
		if (db_exec(&db,query) == 0 && db_fetch(&db) == 0) {
			SQLGetData(db.hstmt, 1, SQL_C_CHAR, (SQLPOINTER) query, sizeof(query), 0);
			strcpy(info->cluster,query);
			dprintf("cluster name: %s\n", info->cluster);
		}
		db_fetch_done(&db);
		db_disconnect(&db);

//		dprintf("cluster(1): %s\n", info->cluster);
		if (strlen(info->cluster)) {
			for(p = info->cluster; *p; p++) {
				if (!isalnum(*p)) *p = ' ';
			}
//			dprintf("cluster(2): %s\n", info->cluster);
//			strcpy(info->cluster,stredit(info->cluster,"COMPRESS,UPCASE"));
			strcpy(info->cluster,stredit(info->cluster,"COMPRESS"));
//			dprintf("cluster(3): %s\n", info->cluster);
			for(p = info->cluster; *p; p++) {
				if (*p == ' ') *p = '-';
			}
		}
	}
	}

	/* Empty cluster is valid */
	if (strlen(info->cluster) == 0) strcpy(info->cluster,current_farm);
	if (strlen(info->cluster) == 0) strcpy(info->cluster,"UNKNOWN");
//	dprintf("cluster(4): %s\n", info->cluster);

	return 0;
}

static int get_vcinfo(struct esxpost *info) {
	struct farms_record frec;
	struct hosts_record hrec;
	DB db;
	char save_version[16];

	/* Connect to db */
	dprintf("connecting to db..\n");
//	dprintf("db_name: %s, db_user: %s, db_pass: %s\n", info->db_name, info->db_user, info->db_pass);
	if (!strlen(info->db_name)) {
		printf("error: db_name is empty!\n");
		return 1;
	}
	if (db_connect(&db,info->db_name,info->db_user,info->db_pass)) return 1;

	strcpy(save_version,info->version);
//	hosts_select_by_name(&db,&hrec,info->fqdn);
	memset(&hrec,0,sizeof(hrec));
	current_farm[0] = 0;
	if (hosts_select_by_name(&db,&hrec,info->fqdn) == 0 && farms_select_by_id(&db,&frec,hrec.farm_id) == 0) {
		strcpylc(info->vcserver,frec.server,sizeof(info->vcserver));
//		strcpyuc(info->cluster,frec.name,sizeof(info->cluster));
		strcpy(info->cluster,frec.name);
dprintf("frec.name: %s\n", frec.name);
		strcpy(current_farm,info->cluster);
		strcpy(info->version,hrec.version);
	}
	if (!real) {
		dprintf("host version: %s, db version: %s\n", info->version, hrec.version);
		if (strcmp(info->version,hrec.version) == 0) {
			info->vcserver[0] = info->cluster[0] = 0;
			strcpy(info->version,save_version);
		}
		if (strlen(info->cluster) == 0 && get_cluster(info) != 0) return 1;
		if (strlen(info->vcserver) == 0 && get_vc(info) != 0) return 1;
	}
	get_dc(info);

//strcpy(info->dc,"Houston West Test");
//strcpy(info->cluster,"TEST");
//	if (farm) strcpyuc(info->cluster,farm,sizeof(info->cluster));
	if (farm) strcpy(info->cluster,farm);

	dprintf("vcserver: %s\n", info->vcserver);
	if (!strlen(info->vcserver)) {
		printf("error: unable to determine vcserver!\n");
		return 1;
	}
	dprintf("dc: %s\n", info->dc);
	if (!strlen(info->dc)) {
		printf("error: unable to determine datacenter!\n");
		return 1;
	}
	dprintf("cluster: %s\n", info->cluster);
	if (!strlen(info->cluster)) {
		printf("error: unable to get cluster name!\n");
		return 1;
	}
	db_disconnect(&db);

	/* Correct UNKNOWN cluster to <datacenter>-UNKNOWN (since name is unique) */
	if (strcmp(info->cluster,"UNKNOWN") == 0 && strlen(info->dc) > 0) sprintf(info->cluster,"%s-UNKNOWN",info->dc);
	return 0;
}

static int get_license(struct esxpost *info) {
	FILE *fp;
	char licenses[256],line[256],sexp[32],vexp[16],dexp[64],cexp[128],hexp[64];
	int field;

	sprintf(licenses,"%s/licenses",etcdir);
	dprintf("licenses: %s\n", licenses);
	fp = fopen(licenses,"r");
	if (!fp) {
		/* XXX Not an error?? */
//		perror("fopen");
		return 0;
	}
	while(fgets(line,sizeof(line),fp)) {
		strcpy(line,stredit(line,"TRIM,COMPRESS"));
		dprintf("line: %s\n", line);
		if (line[0] == '#') continue;
		/* format: vers,dc,cluster,host,license */
		field = 0;
		strcpy(vexp,strele(field++,",",line));
		dprintf("version: %s, vexp: %s\n", info->version, vexp);
		if (!match(info->version,vexp)) continue;
		dprintf("version matched.\n");
		strcpy(sexp,strele(field++,",",line));
		dprintf("vcserver: %s, sexp: %s\n", info->vcserver, sexp);
		if (!match(info->vcserver,sexp)) continue;
		dprintf("vcserver matched\n");
		strcpy(dexp,strele(field++,",",line));
		dprintf("dc: %s, dexp: %s\n", info->dc, dexp);
		if (!match(info->dc,dexp)) continue;
		dprintf("dc matched\n");
//		strcpyuc(cexp,strele(field++,",",line),sizeof(cexp));
		strcpy(cexp,strele(field++,",",line));
		dprintf("cluster: %s, cexp: %s\n", info->cluster, cexp);
		if (!match(info->cluster,cexp)) continue;
		dprintf("cluster matched\n");
		strcpylc(hexp,strele(field++,",",line),sizeof(hexp));
		dprintf("host: %s, hexp: %s\n", info->fqdn, hexp);
		if (!match(info->fqdn,hexp)) continue;
		dprintf("host matched\n");
		strcpy(info->license,strele(field++,",",line));
		dprintf("license: %s\n", info->license);
		break;
	}
	fclose(fp);

	return 0;
}

int get_info(struct esxpost *info, char *host) {
	struct hostent *he;
	unsigned char *ptr;
	char temp[256], *p;
	int i;

	/* If IP specified, use it */
	if (hostip) {
		strcpylc(info->fqdn,host,sizeof(info->fqdn));
		info->addr[0] = 0;
		strncat(info->addr,hostip,sizeof(info->addr)-1);
	} else {
		/* Get hostent */
		he = _gethost(host);
		if (he) {
			/* Split out FQDN and get IP addr */
			strcpylc(info->fqdn,he->h_name,sizeof(info->fqdn));
			ptr = (unsigned char *) he->h_addr;
			sprintf(info->addr,"%d.%d.%d.%d",ptr[0],ptr[1],ptr[2],ptr[3]);
		} else {
			printf("error: host: %s not found in dns!\n",host);
			return 1;
		}
	}

	dprintf("fqdn: %s\n", info->fqdn);
	dprintf("addr: %s\n", info->addr);

	p = info->fqdn;
	i = 0;
	while(*p != '.') info->name[i++] = *p++;
	info->name[i] = 0;
	dprintf("name: %s\n", info->name);
	p++;
	i = 0;
	while(*p) info->domain[i++] = *p++;
	dprintf("domain: %s\n", info->domain);

	/* _ALL_ hosts must have [A-z][0-9] */
	if (isalpha(info->name[0]) && isdigit(info->name[1])) {
		/* RCS hosts will have 2 digits */
		if (isdigit(info->name[2]) && isalpha(info->name[3])) {
			temp[0] = 0;
			strncat(temp,&info->name[1],2);
			temp[2] = 0;
			info->site = atoi(temp);
			info->rcs = 1;
		} else {
			if (isalpha(info->name[2])) {
				temp[0] = info->name[1];
				temp[1] = 0;
				info->site = atoi(temp);
			} else {
				printf("error: hostname does not adhere to NGDC/RCS standard!\n");
				return 1;
			}
		}
	}
	dprintf("site: %d, rcs: %d\n", info->site, info->rcs);
	if (info->site > 99 || info->site < 1) info->site = 0;

	if (strlen(info->version) == 0 && get_version(info)) return 1;
	if (get_vcinfo(info)) return 1;
	if (!clear_alarms) {
		if (get_license(info)) return 1;
		if (callsub(info)) return 1;
	}

	return 0;
}
