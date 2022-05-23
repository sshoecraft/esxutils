
#include <stdio.h>
#ifdef __WIN32
#define WIN32_LEAN_AND_MEAN 1
#include <windows.h>
#endif
#include <sqlext.h>
#define STR_LEN 60

int main(void)
{
	HENV     henv;						  /* environment handle */
	HDBC     hdbc;						  /* connection handle */
//	HSTMT    hstmt;						  /* statement handle */
	SDWORD   rc;						  /* return code */
	UCHAR    info[STR_LEN];					  /* info string for SQLGetInfo */

	rc = SQLAllocEnv(&henv);
	rc = SQLAllocConnect(henv, &hdbc);
	rc = SQLConnect(hdbc, "sysadm", SQL_NTS, "sysadm_ro", SQL_NTS, 0, 0);
	if (rc != SQL_SUCCESS) goto done;
	rc = SQLGetInfo(hdbc, SQL_DBMS_VER, &info, STR_LEN, 0);
	if (rc != SQL_SUCCESS) goto done;
	printf("Current DBMS version is %s\n", info);
done:
	SQLDisconnect(hdbc);
	SQLFreeConnect(hdbc);
	SQLFreeEnv(henv);
	return (rc != SQL_SUCCESS);
}
