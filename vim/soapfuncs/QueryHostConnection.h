
#ifndef __VIM_QueryHostConnectionInfo_H
#define __VIM_QueryHostConnectionInfo_H

#include "soapfuncs.h"
#include "ManagedObjectReference.h"
#include "HostConnectInfo.h"

SOAPFUNCDEFIO(QueryHostConnectionInfo,struct ManagedObjectReference *,struct HostConnectInfo *);

#endif
