
#ifndef __VIM_Login_H
#define __VIM_Login_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "UserSession.h"

struct LoginRequest {
	struct ManagedObjectReference *obj;
	char *userName;
	char *password;
	char *locale;
};

SOAPFUNCDEFIO(Login,struct LoginRequest *,struct UserSession *);

#endif
