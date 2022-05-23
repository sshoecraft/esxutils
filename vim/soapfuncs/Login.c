
#include "Login.h"

static int put_login(struct soap *soap, void *arg) {
	struct LoginRequest *req = arg;
	struct mydesc put_login_desc[] = {
		{ "_this", &req->obj, put_ManagedObjectReference, 0 },
		{ "userName", &req->userName, put_char, 0 },
		{ "password", &req->password, put_char, 0 },
		{ 0,0,0,0 }
	};

	dprintf("user: %s, pass: %s\n", req->userName, req->password);
	return soap_send_desc(soap,"Login","xmlns",MYNS,put_login_desc);
}

static int get_login(struct soap *soap, void *arg) {
	struct UserSession *login = arg;
	struct mydesc get_login_desc[] = {
		{ "key", &login->key, get_char, 1 },
		{ "userName", &login->userName, get_char, 1 },
		{ "fullName", &login->fullName, get_char, 1 },
		{ "loginTime", &login->loginTime, get_char, 0 },
		{ "lastActiveTime", &login->lastActiveTime, get_char, 0 },
		{ "locale", &login->locale, get_char, 0 },
		{ "messageLocale", &login->messageLocale, get_char, 0 },
		{ 0,0,0 }
	};

	if (soap_element_begin_in(soap, "LoginResponse", 0, 0)
		|| soap_element_begin_in(soap, "returnval", 0, 0)
		|| soap_recv_desc(soap, 0, get_login_desc)
		|| soap_element_end_in(soap, "returnval")
		|| soap_element_end_in(soap, "LoginResponse"))
		return 1;
	return 0;
}

SOAPFUNCIO(Login,put_login,struct LoginRequest *,get_login,struct UserSession *);
