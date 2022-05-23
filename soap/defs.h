
typedef struct X509_extension_st
  {
  ASN1_OBJECT *object;
  ASN1_BOOLEAN critical;
  ASN1_OCTET_STRING *value;
  } X509_EXTENSION;
