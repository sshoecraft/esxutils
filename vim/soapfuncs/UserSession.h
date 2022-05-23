
#ifndef __VIM_UserSession_H
#define __VIM_UserSession_H

struct UserSession {
	char *dynamicType;
	int dynamicPropertyCount;
	struct DynamicProperty *dynamicProperty;
	char *key;
	char *userName;
	char *fullName;
	time_t loginTime;
	time_t lastActiveTime;
	char *locale;
	char *messageLocale;
};

#endif
