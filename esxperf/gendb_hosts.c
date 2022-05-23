
/*
*** this file is automatically generated - DO NOT MODIFY
*/

#include <stdio.h>
#include <string.h>
#include "gendb_hosts.h"

int hosts_select(DB *db, char *clause) {
#if hosts_field_count > 0
	char query[4096];
	SQLRETURN ret;

	sprintf(query,"SELECT hosts.id,hosts.farm_id,hosts.name,hosts.model,hosts.status,hosts.version,hosts.build,hosts.psp,hosts.cpu_pkgs,hosts.cpu_count,hosts.cpu_total,hosts.cpu_speed,hosts.mem_total,hosts.cons_mem,hosts.subnet,hosts.last_seen,hosts.in_maint FROM hosts %s",(clause ? clause : ""));
	dprintf("Executing query: %s\n", query);
	ret = SQLExecDirect(db->hstmt, (SQLCHAR *) query, SQL_NTS);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) return 0;
	ret = dberr(SQL_HANDLE_STMT,db->hstmt,"hosts_select");
	return ret;
#else
	return 0;
#endif
}

int hosts_select_record(DB *db, struct hosts_record *rec, char *clause) {
#if hosts_field_count > 0
	SQLRETURN ret;

	if ((ret = hosts_select(db, clause)) != 0) return ret;
	if ((ret = hosts_fetch_record(db, rec)) != 0) return ret;
	SQLCloseCursor(db->hstmt);
#endif
	return 0;
}

int hosts_insert(DB *db, struct hosts_record *rec) {
#if hosts_field_count > 0
	char query[4096];
	SQLRETURN ret;

	sprintf(query,"INSERT INTO hosts (farm_id,name,model,status,version,build,psp,cpu_pkgs,cpu_count,cpu_total,cpu_speed,mem_total,cons_mem,subnet,in_maint) VALUES (%d,'%s','%s','%s','%s',%d,%f,%d,%d,%d,%d,%d,%d,'%s','%s')",rec->farm_id,rec->name,rec->model,rec->status,rec->version,rec->build,rec->psp,rec->cpu_pkgs,rec->cpu_count,rec->cpu_total,rec->cpu_speed,rec->mem_total,rec->cons_mem,rec->subnet,rec->in_maint);
	dprintf("Executing query: %s\n", query);
	ret = SQLExecDirect(db->hstmt, (SQLCHAR *) query, SQL_NTS);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) return 0;
	ret = dberr(SQL_HANDLE_STMT,db->hstmt,"hosts_insert");
	printf("Offending query: %s\n", query);
	return ret;
#else
	return 0;
#endif
}

int hosts_update_record(DB *db, struct hosts_record *rec, char *clause) {
#if hosts_field_count > 0
	char query[4096];
	SQLRETURN ret;

	sprintf(query,"UPDATE hosts SET farm_id = %d,model = '%s',status = '%s',version = '%s',build = %d,psp = %f,cpu_pkgs = %d,cpu_count = %d,cpu_total = %d,cpu_speed = %d,mem_total = %d,cons_mem = %d,subnet = '%s',in_maint = '%s'",rec->farm_id,rec->model,rec->status,rec->version,rec->build,rec->psp,rec->cpu_pkgs,rec->cpu_count,rec->cpu_total,rec->cpu_speed,rec->mem_total,rec->cons_mem,rec->subnet,rec->in_maint);
	if (clause) {
		if (clause[0] != ' ') strcat(query," ");
		strcat(query,clause);
	}
	dprintf("Executing query: %s\n", query);
	ret = SQLExecDirect(db->hstmt, (SQLCHAR *) query, SQL_NTS);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) return 0;
	ret = dberr(SQL_HANDLE_STMT,db->hstmt,"hosts_update_record");
	return ret;
#else
	return 0;
#endif
}

int hosts_fetch(DB *db) {
#if hosts_field_count > 0
	SQLRETURN ret;

	dprintf("Fetching data...\n");
	ret = SQLFetch(db->hstmt);
	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) return 0;
	if (ret != 100) ret = dberr(SQL_HANDLE_STMT,db->hstmt,"hosts_fetch");
	return ret;
#else
	return 0;
#endif
}

int hosts_fetch_record(DB *db, struct hosts_record *rec) {
#if hosts_field_count > 0
	SQLRETURN ret;

	if ((ret = hosts_fetch(db)) != 0) {
		SQLCloseCursor(db->hstmt);
		return ret;
	}
	memset(rec,0,sizeof(*rec));
	SQLGetData(db->hstmt, 1, SQL_C_SLONG, (SQLPOINTER) &rec->id, sizeof(rec->id), 0);
	SQLGetData(db->hstmt, 2, SQL_C_SLONG, (SQLPOINTER) &rec->farm_id, sizeof(rec->farm_id), 0);
	SQLGetData(db->hstmt, 3, SQL_C_CHAR, (SQLPOINTER) rec->name, sizeof(rec->name), 0);
	SQLGetData(db->hstmt, 4, SQL_C_CHAR, (SQLPOINTER) rec->model, sizeof(rec->model), 0);
	SQLGetData(db->hstmt, 5, SQL_C_CHAR, (SQLPOINTER) rec->status, sizeof(rec->status), 0);
	SQLGetData(db->hstmt, 6, SQL_C_CHAR, (SQLPOINTER) rec->version, sizeof(rec->version), 0);
	SQLGetData(db->hstmt, 7, SQL_C_SLONG, (SQLPOINTER) &rec->build, sizeof(rec->build), 0);
	SQLGetData(db->hstmt, 8, SQL_C_FLOAT, (SQLPOINTER) &rec->psp, sizeof(rec->psp), 0);
	SQLGetData(db->hstmt, 9, SQL_C_SLONG, (SQLPOINTER) &rec->cpu_pkgs, sizeof(rec->cpu_pkgs), 0);
	SQLGetData(db->hstmt, 10, SQL_C_SLONG, (SQLPOINTER) &rec->cpu_count, sizeof(rec->cpu_count), 0);
	SQLGetData(db->hstmt, 11, SQL_C_SLONG, (SQLPOINTER) &rec->cpu_total, sizeof(rec->cpu_total), 0);
	SQLGetData(db->hstmt, 12, SQL_C_SLONG, (SQLPOINTER) &rec->cpu_speed, sizeof(rec->cpu_speed), 0);
	SQLGetData(db->hstmt, 13, SQL_C_SLONG, (SQLPOINTER) &rec->mem_total, sizeof(rec->mem_total), 0);
	SQLGetData(db->hstmt, 14, SQL_C_SLONG, (SQLPOINTER) &rec->cons_mem, sizeof(rec->cons_mem), 0);
	SQLGetData(db->hstmt, 15, SQL_C_CHAR, (SQLPOINTER) rec->subnet, sizeof(rec->subnet), 0);
	SQLGetData(db->hstmt, 16, SQL_C_CHAR, (SQLPOINTER) rec->last_seen, sizeof(rec->last_seen), 0);
	SQLGetData(db->hstmt, 17, SQL_C_CHAR, (SQLPOINTER) rec->in_maint, sizeof(rec->in_maint), 0);
	dprintf("----------------- hosts record -----------------\n");
	dprintf("id                  :  %d\n",rec->id);
	dprintf("farm_id             :  %d\n",rec->farm_id);
	dprintf("name                :  %s\n",rec->name);
	dprintf("model               :  %s\n",rec->model);
	dprintf("status              :  %s\n",rec->status);
	dprintf("version             :  %s\n",rec->version);
	dprintf("build               :  %d\n",rec->build);
	dprintf("psp                 :  %f\n",rec->psp);
	dprintf("cpu_pkgs            :  %d\n",rec->cpu_pkgs);
	dprintf("cpu_count           :  %d\n",rec->cpu_count);
	dprintf("cpu_total           :  %d\n",rec->cpu_total);
	dprintf("cpu_speed           :  %d\n",rec->cpu_speed);
	dprintf("mem_total           :  %d\n",rec->mem_total);
	dprintf("cons_mem            :  %d\n",rec->cons_mem);
	dprintf("subnet              :  %s\n",rec->subnet);
	dprintf("last_seen           :  %s\n",rec->last_seen);
	dprintf("in_maint            :  %s\n",rec->in_maint);
#endif
	return 0;
}

