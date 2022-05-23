
#include <termios.h>
#include "ivim.h"

#include "RetrieveServiceContent.h"
#include "Login.h"
#include "Logout.h"

#include "encode.h"
#include "encrypt.h"

char *empty_str = "";

SOAP_NMAC struct Namespace namespaces[] =
{
        {"SOAP-ENV", "http://schemas.xmlsoap.org/soap/envelope/", "http://www.w3.org/*/soap-envelope", NULL},
        {"SOAP-ENC", "http://schemas.xmlsoap.org/soap/encoding/", "http://www.w3.org/*/soap-encoding", NULL},
        {"xsi", "http://www.w3.org/2001/XMLSchema-instance", "http://www.w3.org/*/XMLSchema-instance", NULL},
        {"xsd", "http://www.w3.org/2001/XMLSchema", "http://www.w3.org/*/XMLSchema", NULL},
        {"ns2", "urn:vim25", NULL, NULL},
        {NULL, NULL, NULL, NULL}
};

int vim_getpass(char *password, int szpass) {
    struct termios oflags, nflags;

    /* disabling echo */
    tcgetattr(fileno(stdin), &oflags);
    nflags = oflags;
    nflags.c_lflag &= ~ECHO;
    nflags.c_lflag |= ECHONL;

    if (tcsetattr(fileno(stdin), TCSANOW, &nflags) != 0) {
        perror("tcsetattr");
        return EXIT_FAILURE;
    }

    printf("Password: ");
    fgets(password, szpass, stdin);
	password[ strcspn( password, "\n" ) ] = '\0';

    /* restore terminal */
    if (tcsetattr(fileno(stdin), TCSANOW, &oflags) != 0) {
        perror("tcsetattr");
        return EXIT_FAILURE;
    }

	return strlen(password);
}

static struct vim_session *vim_create_session(void) {
	struct vim_session *s;
	struct soap *soap;

	s = (struct vim_session *) malloc(sizeof(struct vim_session));
	if (!s) {
//		strcpy(_vim_errorstr,"unable to create session: memory allocation failure");
		perror("unable to create vim session: memory allocation failure");
		return 0;
	}
	memset(s,0,sizeof(*s));

	soap = soap_new();
	soap_init(soap);

	soap_ssl_init();
	if (soap_ssl_client_context(soap, SOAP_SSL_NO_AUTHENTICATION, NULL, NULL, NULL, NULL, NULL)) {
		dprintf("error setting ssl_client_context!\n");
		free(s);
		return 0;
	}

	/* Set keepalive */
	soap_set_mode(soap,SOAP_IO_KEEPALIVE);

	/* http://www.mail-archive.com/gsoap@yahoogroups.com/msg01124.html */
	/* XXX any value for connect_timeout will mean a 10 sec SSL_connect timeout, might as well make em the same */
	soap->connect_timeout = 10;

	s->soap = soap;
	return s;
}

struct vim_session *vim_connect(char *server, int port) {
	struct vim_session *s;

	s = vim_create_session();
	if (!s) return 0;

	dprintf("server: %s, port: %d\n", server, port);
	if (port)
		snprintf(s->endpoint,sizeof(s->endpoint),"https://%s:%d/sdk", server, port);
	else
		snprintf(s->endpoint,sizeof(s->endpoint),"https://%s/sdk", server);
	dprintf("endpoint: %s\n", s->endpoint);
	soap_set_endpoint(s->soap, s->endpoint);

	dprintf("connecting...\n");
	if (soap_connect(s->soap,s->endpoint,MYNS) != SOAP_OK) {
#ifdef DEBUG
		soap_print_fault(s->soap,stdout);
#endif
		dprintf("error connecting!\n");
		goto error;
	}
	strcpy(s->server,server);
	s->port = port;

	s->sc = soap_malloc(s->soap, sizeof(*s->sc));
	memset(s->sc,0,sizeof(*s->sc));
	s->info = soap_malloc(s->soap, sizeof(*s->info));
	memset(s->info, 0, sizeof(*s->info));

	if (RetrieveServiceContent(s->soap, s->endpoint, s->sc)) {
		dprintf("error getting ServiceContent!\n");
		goto error;
	}

	return s;

error:
//	soap_done(s->soap);
	soap_end(s->soap);
	free(s);
	return 0;

}

int vim_login(struct vim_session *s, char *user, char *pass) {
	struct LoginRequest req;
//	char username[32];

//	printf("s: %p, user: %p, pass: %p\n", s, user, pass);
	if (!s) {
		printf("error: vim_login: session ptr is null!");
		return 1;
	}
//	CHECKSC(s);
	if (!s->sc || !s->sc->sessionManager) {
		printf("error: vim_login: sessionManager is null!");
		return 1;
	}

	dprintf("user: %s\n", user);
	if (!user) {
		printf("error: vim_login: user is null!");
		return 1;
	}
	if (!pass) {
		pass = vim_cred_get(s->server, user);
		if (!pass) {
			printf("vim_login: no pass specified or found in store\n.");
			return 1;
		}
	}
	dprintf("pass: %s\n", pass);

//	printf("user: %s, pass: %s\n", user, pass);
	req.obj = s->sc->sessionManager;
	req.userName = user;
	req.password = pass;
	req.locale = NULL;

	if (Login(s->soap, s->endpoint, &req, s->info) != SOAP_OK) {
		dprintf("login failed!\n");
	#ifdef DEBUG
		soap_print_fault(s->soap,stdout);
#endif
		return 1;
	}
	dprintf("fullName: %s\n", s->info->fullName);

	return 0;
}

static char str[64];
static int idx = 0;

static int _getstr(void *arg, int ch) {
        if (ch >= 0 && idx < sizeof(str)-1) str[idx++] = ch;
        return 0;
}

int vim_secure_login(struct vim_session *s, char *user, char *pass) {
	struct encoder_context decode_ctx;
	struct encryptor_context decrypt_ctx;
	char *p;

	if (!user || !pass) {
		printf("vim_secure_login requires both user and pass\n");
		return 1;
	}

	encoder_init(&decode_ctx, decrypt_byte, &decrypt_ctx);
	encryptor_init(&decrypt_ctx, _getstr, 0, (unsigned char *)user, strlen(user));

	idx = 0;
	for(p=pass; *p; p++) decode_byte(&decode_ctx, *p);
	decode_byte(&decode_ctx, -1);
//	dprintf("str: %s\n", str);

	return vim_login(s,user,str);
}

int vim_logout(struct vim_session *s) {
//	CHECKSC(s);

	if(Logout(s->soap,s->endpoint,s->sc->sessionManager) != SOAP_OK) {
		dprintf("error logging out!\n");
		return 1;
	}

	return 0;
}

void vim_disconnect(struct vim_session *s) {
	if (s->info && s->info->key) {
		dprintf("calling Logout...\n");
		vim_logout(s);
	}
//	dprintf("calling soap_done...\n");
//	soap_done(s->soap);
	dprintf("calling soap_end...\n");
	soap_end(s->soap);
	memset(s,0,sizeof(*s));
	free(s);
	return;
}
