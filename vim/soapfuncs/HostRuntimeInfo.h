
#ifndef __VIM_HostRuntimeInfo_h
#define __VIM_HostRuntimeInfo_h

struct HostRuntimeInfo {
//	datetime_t *bootTime;
	char *bootTime;
	char *connectionState;
	bool inMaintenanceMode;
};

#endif
