
#ifndef __VIM_TaskInfo_H
#define __VIM_TaskInfo_H

#include "list.h"
#include "soapfuncs.h"
#include "ManagedObjectReference.h"

struct TaskInfo {
	char *key;
	struct ManagedObjectReference *task;
	char *name;
	char *description;
	char *descriptionId;
	struct ManagedObjectReference *entity;
	char *entityName;
	char *state;
	list reason;
	int progress;
	bool cancelled;
	bool cancelable;
	char *fault;
	char *faultMessage;
	char *userName;
	char *queueTime;
	char *startTime;
	char *completeTime;
	int eventChainId;
#if 0
	int __sizelocked;       /* sequence of elements <locked> */
	struct ManagedObjectReference *locked;     /* optional element of type ns2:ManagedObjectReference */
        char *result;   /* optional element of type xsd:anyType */
        int *progress;  /* optional element of type xsd:int */
//        struct TaskReason *reason; /* required element of type ns2:TaskReason */
        time_t queueTime;       /* required element of type xsd:dateTime */
        time_t *startTime;      /* optional element of type xsd:dateTime */
        time_t *completeTime;   /* optional element of type xsd:dateTime */
        int eventChainId;       /* required element of type xsd:int */
        char *changeTag;        /* optional element of type xsd:string */
        char *parentTaskKey;    /* optional element of type xsd:string */
        char *rootTaskKey;      /* optional element of type xsd:string */
#endif
};

int get_taskInfo(struct soap *soap, char *tag, list results);

#endif
