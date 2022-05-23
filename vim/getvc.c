
#include "ivim.h"
#include "util.h"

#define VCSERVERS "/home/esxadmin/etc/vcservers"

/* Match version, cluster & host, return server, port, user */
int get_vcserver(char *version, char *cluster, char *host, char *server, int port, char *user) {
	FILE *fp;
	char line[1024],vexp[32],cexp[128],hexp[64];
	int found;

//	dprintf("vcservers: %s\n", VCSERVERS);
	fp = fopen(VCSERVERS,"r");
	if (!fp) {
		printf("error: unable to read vcservers file (%s)!\n", VCSERVERS);
		return 1;
	}
	found = 0;
	/* Line format: Hostexp,Server */
	/* fomat: Version,Cluster,Host,Server:Port */
	while(fgets(line,sizeof(line),fp)) {
		strcpy(line,stredit(line,"TRIM,COMPRESS"));
		dprintf("line: %s\n", line);
		if (line[0] == '#') continue;
		strcpy(vexp,strele(0,",",line));
		if (version && !match(version,vexp)) continue;
		dprintf("version matched.\n");
		strcpy(cexp,strele(1,",",line));
		if (cluster && !match(cluster,cexp)) continue;
		dprintf("cluster matched.\n");
		strcpy(hexp,strele(2,",",line));
		if (host && !match(host,hexp)) continue;
		dprintf("host matched.\n");
		strcpy(user,strele(4,",",line));
		strcpy(line,strele(3,",",line));
		strcpy(server,strele(0,":",line));
		port = atoi(strele(1,":",line));
		if (!port) port = 901;
		dprintf("server: %s, port: %d, user: %s\n", server, port, user);
		found = 1;
		break;
	}
	fclose(fp);

	dprintf("found: %d\n", found);
	return (found == 0);
}

/* Match server & port, return user */
int get_vcuser(char *server, int port, char *user) {
	FILE *fp;
	char line[1024],sexp[256], spec[256];
	int found;

	strcpy(spec,server);
	if (port) {
		char temp[16];

		sprintf(temp,":%d",port);
		strcat(spec,temp);
	}
	dprintf("spec: %s\n", spec);

	fp = fopen(VCSERVERS,"r");
	if (!fp) {
		printf("error: unable to read vcservers file (%s)!\n", VCSERVERS);
		return 1;
	}
	found = 0;
	/* fomat: Version,Cluster,Host,Server:Port,User */
	while(fgets(line,sizeof(line),fp)) {
		strcpy(line,stredit(line,"TRIM,COMPRESS"));
		dprintf("line: %s\n", line);
		if (line[0] == '#') continue;
#if 0
		strcpy(vexp,strele(0,",",line));
		if (version && !match(version,vexp)) continue;
		dprintf("version matched.\n");
		strcpy(cexp,strele(1,",",line));
		if (cluster && !match(cluster,cexp)) continue;
		dprintf("cluster matched.\n");
		strcpy(hexp,strele(2,",",line));
		if (host && !match(host,hexp)) continue;
		dprintf("host matched.\n");
#endif
		strcpy(sexp,strele(3,",",line));
		dprintf("spec: %s, sexp: %s\n", spec, sexp);
//		if (!match(sexp,spec)) continue;
		if (!match(spec,sexp)) continue;
		strcpy(user,strele(4,",",line));
#if 0
		strcpy(server,strele(0,":",sexp));
		port = atoi(strele(1,":",sexp));
		if (!port) port = 901;
		dprintf("server: %s, port: %d, user: %s\n", server, port, user);
#endif
		dprintf("user: %s\n", user);
		found = 1;
		break;
	}
	fclose(fp);

	dprintf("found: %d\n", found);
	return (found == 0);
}
