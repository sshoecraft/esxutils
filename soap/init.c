
#include "stdsoap2.h"

SOAP_FMAC1
void
SOAP_FMAC2
soap_init(struct soap *soap)
{ soap->state = SOAP_INIT;
#ifdef SOAP_MEM_DEBUG
  soap_init_mht(soap);
#endif
#if !defined(WITH_LEAN) || defined(SOAP_DEBUG)
  soap_init_logs(soap);
#endif
#ifdef SOAP_DEBUG
  soap_set_test_logfile(soap, "TEST.log");
  soap_set_sent_logfile(soap, "SENT.log");
  soap_set_recv_logfile(soap, "RECV.log");
#endif
  soap->version = 0;
  soap_imode(soap, SOAP_IO_DEFAULT);
  soap_omode(soap, SOAP_IO_DEFAULT);
  soap->plugins = NULL;
  soap->user = NULL;
  soap->userid = NULL;
  soap->passwd = NULL;
#ifndef WITH_NOHTTP
  soap->fpost = http_post;
  soap->fget = http_get;
  soap->fput = http_405;
  soap->fdel = http_405;
  soap->fhead = http_405;
  soap->fform = NULL;
  soap->fposthdr = http_post_header;
  soap->fresponse = http_response;
  soap->fparse = http_parse;
  soap->fparsehdr = http_parse_header;
#endif
  soap->fheader = NULL;
  soap->fconnect = NULL;
  soap->fdisconnect = NULL;
#ifndef WITH_NOIO
  soap->ipv6_multicast_if = 0;
  soap->ipv4_multicast_if = NULL;
#ifndef WITH_IPV6
  soap->fresolve = tcp_gethost;
#else
  soap->fresolve = NULL;
#endif
  soap->faccept = tcp_accept;
  soap->fopen = tcp_connect;
  soap->fclose = tcp_disconnect;
  soap->fclosesocket = tcp_closesocket;
  soap->fshutdownsocket = tcp_shutdownsocket;
  soap->fsend = fsend;
  soap->frecv = frecv;
  soap->fpoll = soap_poll;
#else
  soap->fopen = NULL;
  soap->fclose = NULL;
  soap->fpoll = NULL;
#endif
  soap->fseterror = NULL;
  soap->fignore = NULL;
  soap->fserveloop = NULL;
  soap->fplugin = fplugin;
  soap->fmalloc = NULL;
#ifndef WITH_LEANER
  soap->fprepareinitsend = NULL;
  soap->fprepareinitrecv = NULL;
  soap->fpreparesend = NULL;
  soap->fpreparerecv = NULL;
  soap->fpreparefinalsend = NULL;
  soap->fpreparefinalrecv = NULL;
  soap->fdimereadopen = NULL;
  soap->fdimewriteopen = NULL;
  soap->fdimereadclose = NULL;
  soap->fdimewriteclose = NULL;
  soap->fdimeread = NULL;
  soap->fdimewrite = NULL;
  soap->fmimereadopen = NULL;
  soap->fmimewriteopen = NULL;
  soap->fmimereadclose = NULL;
  soap->fmimewriteclose = NULL;
  soap->fmimeread = NULL;
  soap->fmimewrite = NULL;
#endif
  soap->float_format = "%.9G"; /* Alternative: use "%G" */
  soap->double_format = "%.17lG"; /* Alternative: use "%lG" */
  soap->dime_id_format = "cid:id%d"; /* default DIME id format */
  soap->http_version = "1.1";
  soap->proxy_http_version = "1.0";
  soap->http_content = NULL;
  soap->actor = NULL;
  soap->lang = "en";
  soap->keep_alive = 0;
  soap->tcp_keep_alive = 0;
  soap->tcp_keep_idle = 0;
  soap->tcp_keep_intvl = 0;
  soap->tcp_keep_cnt = 0;
  soap->max_keep_alive = SOAP_MAXKEEPALIVE;
  soap->recv_timeout = 0;
  soap->send_timeout = 0;
  soap->connect_timeout = 0;
  soap->accept_timeout = 0;
  soap->socket_flags = 0;
  soap->connect_flags = 0;
  soap->bind_flags = 0;
  soap->accept_flags = 0;
  soap->linger_time = 0;
  soap->ip = 0;
  soap->labbuf = NULL;
  soap->lablen = 0;
  soap->labidx = 0;
  soap->encodingStyle = SOAP_STR_EOS;
#ifndef WITH_NONAMESPACES
  soap->namespaces = namespaces;
#else
  soap->namespaces = NULL;
#endif
  soap->local_namespaces = NULL;
  soap->nlist = NULL;
  soap->blist = NULL;
  soap->clist = NULL;
  soap->alist = NULL;
  soap->attributes = NULL;
  soap->header = NULL;
  soap->fault = NULL;
  soap->master = SOAP_INVALID_SOCKET;
  soap->socket = SOAP_INVALID_SOCKET;
  soap->os = NULL;
  soap->is = NULL;
#ifndef WITH_LEANER
  soap->dom = NULL;
  soap->dime.list = NULL;
  soap->dime.first = NULL;
  soap->dime.last = NULL;
  soap->mime.list = NULL;
  soap->mime.first = NULL;
  soap->mime.last = NULL;
  soap->mime.boundary = NULL;
  soap->mime.start = NULL;
  soap->xlist = NULL;
#endif
#ifndef UNDER_CE
  soap->recvfd = 0;
  soap->sendfd = 1;
#else
  soap->recvfd = stdin;
  soap->sendfd = stdout;
#endif 
  soap->host[0] = '\0';
  soap->port = 0;
  soap->action = NULL;
  soap->proxy_host = NULL;
  soap->proxy_port = 8080;
  soap->proxy_userid = NULL;
  soap->proxy_passwd = NULL;
  soap->authrealm = NULL;
  soap->prolog = NULL;
#ifdef WITH_ZLIB
  soap->zlib_state = SOAP_ZLIB_NONE;
  soap->zlib_in = SOAP_ZLIB_NONE;
  soap->zlib_out = SOAP_ZLIB_NONE;
  soap->d_stream = (z_stream*)SOAP_MALLOC(soap, sizeof(z_stream));
  soap->d_stream->zalloc = Z_NULL;
  soap->d_stream->zfree = Z_NULL;
  soap->d_stream->opaque = Z_NULL;
  soap->z_buf = NULL;
  soap->z_level = 6;
  soap->z_dict = NULL;
  soap->z_dict_len = 0;
#endif
#ifndef WITH_LEAN
  soap->wsuid = NULL;
  soap->c14nexclude = NULL;
  soap->cookies = NULL;
  soap->cookie_domain = NULL;
  soap->cookie_path = NULL;
  soap->cookie_max = 32;
#endif
#ifdef WMW_RPM_IO
  soap->rpmreqid = NULL;
#endif
#ifdef PALM
  palmNetLibOpen();
#endif
#ifndef WITH_NOIDREF
  soap_init_iht(soap);
  soap_init_pht(soap);
#endif
#ifdef WITH_OPENSSL
  if (!soap_ssl_init_done)
    soap_ssl_init();
  soap->fsslauth = ssl_auth_init;
  soap->fsslverify = ssl_verify_callback;
  soap->bio = NULL;
  soap->ssl = NULL;
  soap->ctx = NULL;
  soap->ssl_flags = SOAP_SSL_DEFAULT;
  soap->keyfile = NULL;
  soap->password = NULL;
  soap->dhfile = NULL;
  soap->cafile = NULL;
  soap->capath = NULL;
  soap->crlfile = NULL;
  soap->randfile = NULL;
  soap->session = NULL;
#endif
#ifdef WITH_C_LOCALE
  soap->c_locale = newlocale(LC_ALL_MASK, "C", NULL);
#else
  soap->c_locale = NULL;
#endif
  soap->buflen = 0;
  soap->bufidx = 0;
#ifndef WITH_LEANER
  soap->dime.chunksize = 0;
  soap->dime.buflen = 0;
#endif
  soap->null = 0;
  soap->position = 0;
  soap->encoding = 0;
  soap->mustUnderstand = 0;
  soap->ns = 0;
  soap->part = SOAP_END;
  soap->alloced = 0;
  soap->count = 0;
  soap->length = 0;
  soap->cdata = 0;
  soap->peeked = 0;
  soap->ahead = 0;
  soap->idnum = 0;
  soap->level = 0;
  soap->endpoint[0] = '\0';
  soap->error = SOAP_OK;
}
