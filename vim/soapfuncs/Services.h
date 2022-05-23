
struct ns1__StartServiceRequestType
{
        struct ns1__ManagedObjectReference *_USCOREthis;        /* required element of type ns1:ManagedObjectReference */
        char *id;       /* required element of type xsd:string */
};
#endif

#ifndef SOAP_TYPE_ns1__StopServiceRequestType
#define SOAP_TYPE_ns1__StopServiceRequestType (1251)
/* ns1:StopServiceRequestType */
struct ns1__StopServiceRequestType
{
        struct ns1__ManagedObjectReference *_USCOREthis;        /* required element of type ns1:ManagedObjectReference */
        char *id;       /* required element of type xsd:string */
};

void
