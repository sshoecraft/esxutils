
#ifndef __SOAPFUNCS_H
#define __SOAPFUNCS_H

#include <stdsoap2.h>

#ifndef _cplusplus
#ifndef _BOOL_DEFINED
typedef char bool;
#define _BOOL_DEFINED 1
#endif
#endif
#define MYNS "urn:vim25"

/* Function defs */
#define SOAPFUNCDEFI(name,get_arg_type) \
	int name(struct soap *soap, char *endpoint, get_arg_type request)
#define SOAPFUNCI(name,put_func,get_func,get_arg_type) \
int name(struct soap *soap, char *endpoint, get_arg_type reply) { \
	return (soap_send_req(soap,endpoint,put_func,0) || soap_recv_reply(soap,get_func,reply)); }

#define SOAPFUNCDEFO(name,put_arg_type) \
	int name(struct soap *soap, char *endpoint, put_arg_type reply)
#define SOAPFUNCO(name,put_func,put_arg_type,get_func) \
int name(struct soap *soap, char *endpoint, put_arg_type request) { \
	return (soap_send_req(soap,endpoint,put_func,request) || soap_recv_reply(soap,get_func,0)); }

#define SOAPFUNCDEFIO(name,put_arg_type,get_arg_type) \
	int name(struct soap *soap, char *endpoint, put_arg_type request, get_arg_type reply);
#define SOAPFUNCIO(name,put_func,put_arg_type,get_func,get_arg_type) \
int name(struct soap *soap, char *endpoint, put_arg_type request, get_arg_type reply) { \
	return (soap_send_req(soap,endpoint,put_func,request) || soap_recv_reply(soap,get_func,reply)); }

/*
 * Base utils
*/

extern int soap_send_string(struct soap *soap, char *tag, char *);
extern int soap_recv_string(struct soap *soap, char *tag, char **, int);
extern int soap_send_int(struct soap *soap, char *tag, int);
extern int soap_recv_int(struct soap *soap, char *tag, int *, int);
extern int soap_recv_bool(struct soap *soap, char *tag, bool *, int);
extern int soap_send_bool(struct soap *soap, char *tag, bool value);
typedef int (soapfunc_t)(struct soap *, void *);
extern int soap_send_req(struct soap *soap, char *endpoint, soapfunc_t *func, void *arg);
extern int soap_recv_reply(struct soap *soap, soapfunc_t *func, void *arg);

/*
 * Descriptor
*/

typedef int (*process_desc_func_t)(struct soap *, char *, void **, int);
struct mydesc {
	char *tag;
	void *dest;
	process_desc_func_t func;
	int req;
};

int get_DynamicProperty(struct soap *soap, char *tag, void **ptr, int req);
int get_DynamicData(struct soap *soap, char *tag, void **ptr, int req);
int put_OptionValue(struct soap *soap, char *tag, void **ptr, int req);
//int put_HostConnectSpec(struct soap *soap, char *tag, void **ptr, int req);
int get_obj(struct soap *soap, char *tag, void **ptr, int req);
int put_obj(struct soap *soap, char *tag, void **ptr, int req);
int get_char(struct soap *soap, char *tag, void **ptr, int req);
int put_char(struct soap *soap, char *tag, void **ptr, int req);
#define get_time_t get_char
#define put_time_t put_char
int get_int(struct soap *soap, char *tag, void **ptr, int req);
int put_int(struct soap *soap, char *tag, void **ptr, int req);
int get_bool(struct soap *soap, char *tag, void **ptr, int req);
int put_bool(struct soap *soap, char *tag, void **ptr, int req);
extern int soap_send_desc(struct soap *soap, char *tag, char *attr, char *type, struct mydesc *desc);
extern int soap_recv_desc(struct soap *soap, char *tag, struct mydesc *desc);

extern int dpeek(struct soap *);

#ifndef dprintf
#ifdef DEBUG
#define dprintf(format, args...) printf("%s(%d): " format,__FUNCTION__,__LINE__, ## args)
#else
#define dprintf(format, args...) /* noop */
#endif
#endif

#endif
