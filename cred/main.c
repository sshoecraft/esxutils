
#include "vim.h"
#include "util.h"
//#include <crypt.h>
#include <openssl/md5.h>
#include <openssl/evp.h>
struct evp_md_ctx_st {
    const EVP_MD *reqdigest;    /* The original requested digest */
    const EVP_MD *digest;
    ENGINE *engine;             /* functional reference if 'digest' is
                                 * ENGINE-provided */
    unsigned long flags;
    void *md_data;
    /* Public key context for sign/verify */
    EVP_PKEY_CTX *pctx;
    /* Update function: usually copied from EVP_MD */
    int (*update) (EVP_MD_CTX *ctx, const void *data, size_t count);

    /*
     * Opaque ctx returned from a providers digest algorithm implementation
     * OSSL_FUNC_digest_newctx()
     */
    void *algctx;
    EVP_MD *fetched_digest;
} /* EVP_MD_CTX */ ;

typedef struct evp_md_ctx_st EVP_MD_CTX;

#ifdef __WIN32__
//#include <userenv.h>
#include "getopt.h"
extern char *getpass(char *);
#endif

#include "encode.c"

void usage(void) {
	printf("usage: vim_cred [-adgh] [-s <server>[:<port>]] [-u <username> -p <password>]\n");
	printf("  where:\n");
	printf("    -a             add\n");
	printf("    -d             delete\n");
	printf("    -g             specify global operation (requires root)\n");
	printf("    -s             server/port to connect to\n");
	printf("    -u             username\n");
	printf("    -p             password\n");
	printf("    -h             this listing\n");
	printf("    -e             get encoded pass (suitable for chpasswd)\n");
	exit(1);
}

static unsigned const char cov_2char[64]={
        /* from crypto/des/fcrypt.c */
        0x2E,0x2F,0x30,0x31,0x32,0x33,0x34,0x35,
        0x36,0x37,0x38,0x39,0x41,0x42,0x43,0x44,
        0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,
        0x4D,0x4E,0x4F,0x50,0x51,0x52,0x53,0x54,
        0x55,0x56,0x57,0x58,0x59,0x5A,0x61,0x62,
        0x63,0x64,0x65,0x66,0x67,0x68,0x69,0x6A,
        0x6B,0x6C,0x6D,0x6E,0x6F,0x70,0x71,0x72,
        0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7A
};

static char *md5crypt(const char *passwd, const char *magic, const char *salt) {
	static char out_buf[6 + 9 + 24 + 2]; /* "$apr1$..salt..$.......md5hash..........\0" */
	unsigned char buf[MD5_DIGEST_LENGTH];
	char *salt_out;
	int n;
	unsigned int i;
//	EVP_MD_CTX md,md2;
	struct evp_md_ctx_st md,md2;
	size_t passwd_len, salt_len;

	passwd_len = strlen(passwd);
	out_buf[0] = '$';
	out_buf[1] = 0;
	strncat(out_buf, magic, 4);
	strncat(out_buf, "$", 1);
	strncat(out_buf, salt, 8);
	salt_out = out_buf + 2 + strlen(magic);
	salt_len = strlen(salt_out);
	
	EVP_MD_CTX_init(&md);
	EVP_DigestInit_ex(&md,EVP_md5(), NULL);
	EVP_DigestUpdate(&md, passwd, passwd_len);
	EVP_DigestUpdate(&md, "$", 1);
	EVP_DigestUpdate(&md, magic, strlen(magic));
	EVP_DigestUpdate(&md, "$", 1);
	EVP_DigestUpdate(&md, salt_out, salt_len);
	
	EVP_MD_CTX_init(&md2);
	EVP_DigestInit_ex(&md2,EVP_md5(), NULL);
	EVP_DigestUpdate(&md2, passwd, passwd_len);
	EVP_DigestUpdate(&md2, salt_out, salt_len);
	EVP_DigestUpdate(&md2, passwd, passwd_len);
	EVP_DigestFinal_ex(&md2, buf, NULL);

	for (i = passwd_len; i > sizeof buf; i -= sizeof buf)
		EVP_DigestUpdate(&md, buf, sizeof buf);
	EVP_DigestUpdate(&md, buf, i);
	
	n = passwd_len;
	while (n)
		{
		EVP_DigestUpdate(&md, (n & 1) ? "\0" : passwd, 1);
		n >>= 1;
		}
	EVP_DigestFinal_ex(&md, buf, NULL);

	for (i = 0; i < 1000; i++)
		{
		EVP_DigestInit_ex(&md2,EVP_md5(), NULL);
		EVP_DigestUpdate(&md2, (i & 1) ? (unsigned const char *) passwd : buf,
		                       (i & 1) ? passwd_len : sizeof buf);
		if (i % 3)
			EVP_DigestUpdate(&md2, salt_out, salt_len);
		if (i % 7)
			EVP_DigestUpdate(&md2, passwd, passwd_len);
		EVP_DigestUpdate(&md2, (i & 1) ? buf : (unsigned const char *) passwd,
		                       (i & 1) ? sizeof buf : passwd_len);
		EVP_DigestFinal_ex(&md2, buf, NULL);
		}
//	EVP_MD_CTX_cleanup(&md2);
	
	 {
		/* transform buf into output string */
	
		unsigned char buf_perm[sizeof buf];
		int dest, source;
		char *output;

		/* silly output permutation */
		for (dest = 0, source = 0; dest < 14; dest++, source = (source + 6) % 17)
			buf_perm[dest] = buf[source];
		buf_perm[14] = buf[5];
		buf_perm[15] = buf[11];

		output = salt_out + salt_len;
		
		*output++ = '$';

		for (i = 0; i < 15; i += 3)
			{
			*output++ = cov_2char[buf_perm[i+2] & 0x3f];
			*output++ = cov_2char[((buf_perm[i+1] & 0xf) << 2) |
				                  (buf_perm[i+2] >> 6)];
			*output++ = cov_2char[((buf_perm[i] & 3) << 4) |
				                  (buf_perm[i+1] >> 4)];
			*output++ = cov_2char[buf_perm[i] >> 2];
			}
		*output++ = cov_2char[buf_perm[i] & 0x3f];
		*output++ = cov_2char[buf_perm[i] >> 6];
		*output = 0;
	 }
//	EVP_MD_CTX_cleanup(&md);

	return out_buf;
}

int main(int argc, char **argv) {
	char server[64], *user, *pass, *p;
	int ch,port,action,verbose,global;

	server[0] = 0;
	user = pass = 0;
	port = action = verbose = global = 0;
	while((ch = getopt(argc, argv, "adgs:u:p:hvex")) != -1) {
		switch(ch) {
		case 'a':
			if (action == 2) {
				printf("error: -a and -d options are mutually exclusive");
				usage();
			}
			action = 1;
			break;
		case 'd':
			if (action == 1) {
				printf("error: -a and -d options are mutually exclusive");
				usage();
			}
			action = 2;
			break;
		case 'e':
			action = 3;
			break;
		case 'x':
			action = 4;
			break;
		case 'g':
			global = 1;
			break;
		case 's':
			server[0] = 0;
			p = strchr(optarg,':');
			if (p) {
				strncat(server, optarg, p - optarg);
				dprintf("port: %s\n", p);
				port = atoi(p+1);
			} else {
				strncat(server, optarg, sizeof(server)-1);
				port = 0;
			}
			break;
		case 'u':
			user = optarg;
			break;
		case 'p':
			pass = optarg;
			break;
		case 'h':
			usage();
			break;
		case 'v':
			verbose = 1;
			break;
		}
	}
	if (!action) usage();
	if (!strlen(server)) usage();
	if (!user) usage();

	if (port) port=0;
	if (action == 1) {
		/* Add */
		if (!pass) {
			char prompt[128];

			sprintf(prompt,"Enter password for user %s on %s: ",user,server);
			pass = getpass(prompt);
//			dprintf("pass: %p\n", pass);
			if (!pass) goto abort;
//			dprintf("pass: %s\n", pass);
			if (!strlen(pass)) goto abort;
		}
		vim_cred_add(global,server,user,pass);
	} else if (action == 2) {
		/* Delete */
		vim_cred_del(global,server,user);
	} else if (action == 4) {
		printf("%s\n",vim_cred_get(server,user));
	} else {
		char salt[9],seeds[64] = "./0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
		int i;

		pass = vim_cred_get(server, user);
		if (!pass) { printf("creds not found\n"); exit(1); }
		dprintf("pass: %s\n", pass);
		srand((unsigned int)time(NULL));
		for(i=0; i < 8; i++) salt[i] = seeds[rand() % 64];
		salt[8] = 0;
		printf("%s\n",md5crypt(pass,"1",salt));
	}
	return 0;

abort:
	printf("aborted.\n");
	return 1;
}
