
/* XXX only want DEBUG defined if debugging this module */
#ifdef DEBUG
#undef DEBUG
#endif
#define DEBUG 0
#undef DEBUG

#include "ivim.h"
#ifndef __MINGW32__
#include <pwd.h>
#else
#include <windows.h>
#include <userenv.h>
#endif
#include <errno.h>

#ifdef __WIN32
#define GLOBAL_DIR	"C:\\WINDOWS"
#else
#define GLOBAL_DIR	"/usr/local/etc"
#endif

struct store_rec {
	char server[64];
	char user[32];
	char pass[32];
};

typedef struct {
	unsigned char sbox[256];
	int i, j;
} rc4_context_t;

static void rc4_init(rc4_context_t *text,unsigned char *key,unsigned keylen) {
	int i, j;

	/* fill in linearly s0=0 s1=1... */
	for (i=0;i<256;i++)
		text->sbox[i]=i;

	j=0;
	for (i = 0; i < 256; i++) {
		unsigned char tmp;
		/* j = (j + Si + Ki) mod 256 */
		j = (j + text->sbox[i] + key[i % keylen]) % 256;

		/* swap Si and Sj */
		tmp = text->sbox[i];
		text->sbox[i] = text->sbox[j];
		text->sbox[j] = tmp;
	}

	/* counters initialized to 0 */
	text->i = 0;
	text->j = 0;
}

static void rc4_encrypt(rc4_context_t *text,unsigned char *output,unsigned char *input,unsigned input_len) {
	int tmp;
	int i = text->i;
	int j = text->j;
	int t;
	int K;
	unsigned char *input_end = input + input_len;

	while (input < input_end) {
		i = (i + 1) % 256;
		j = (j + text->sbox[i]) % 256;

		/* swap Si and Sj */
		tmp = text->sbox[i];
		text->sbox[i] = text->sbox[j];
		text->sbox[j] = tmp;

		t = (text->sbox[i] + text->sbox[j]) % 256;
		K = text->sbox[t];

		/* byte K is Xor'ed with plaintext */
		*output++ = *input++ ^ K;
	}
	text->i = i;
	text->j = j;
}

static void rc4_decrypt(rc4_context_t *text,char *output,unsigned char *input,unsigned input_len) {
	int tmp;
	int i = text->i;
	int j = text->j;
	int t;
	int K;
	unsigned char *input_end = input + input_len;

	while (input < input_end) {
		i = (i + 1) % 256;
		j = (j + text->sbox[i]) % 256;

		/* swap Si and Sj */
		tmp = text->sbox[i];
		text->sbox[i] = text->sbox[j];
		text->sbox[j] = tmp;

		t = (text->sbox[i] + text->sbox[j]) % 256;
		K = text->sbox[t];

		/* byte K is Xor'ed with plaintext */
		*output++ = *input++ ^ K;
	}
	text->i = i;
	text->j = j;
}


/* Encoder table */
static unsigned char uutable[64] = {
		0x60,0x21,0x22,0x23,0x24,0x25,0x26,0x27,
		0x28,0x29,0x2A,0x2B,0x2C,0x2D,0x2E,0x2F,
		0x30,0x31,0x32,0x33,0x34,0x35,0x36,0x37,
		0x38,0x39,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,
		0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,
		0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F,
		0x50,0x51,0x52,0x53,0x54,0x55,0x56,0x57,
		0x58,0x59,0x5A,0x5B,0x5C,0x5D,0x5E,0x5F
};

#define ENC(c) uutable[(c) & 0x3F]
#define DEC(c) (((c) - 0x20) & 0x3F)

static int encode(char *out, unsigned char *in, int in_len) {
	unsigned char imask, omask;
	unsigned char ich,och;
	int i,j;

	imask = omask = 0x01;
	i = och = 0;
	for(j=0; j < in_len; j++) {
		ich = in[j];
		for(imask=1; imask; imask <<= 1) {
			if (ich & imask) och |= omask;
			omask <<= 1;
			if (omask == 0x40) {
				out[i++] = ENC(och);
				och = 0;
				omask = 1;
			}
		}
	}
	if (och) out[i++] = ENC(och);
	out[i++] = 0;

	return i;
}

static int decode(unsigned char *out, char *in) {
	unsigned char imask, omask;
	unsigned char ich,och;
	char *p;
	int i;

	i = 0;
	imask = omask = 0x01;
	ich = och = 0;
	for (p=in; *p; p++) {
		ich = DEC(*p);
		for(imask=1; imask < 0x40; imask <<= 1) {
			if (ich & imask) och |= omask;
			omask <<= 1;
			if (!omask) {
				out[i++] = och;
				och = 0;
				omask = 1;
			}
		}
	}
	if (och) out[i++] = och;
	out[i] = 0;

	return i;
}

typedef int (storeop_t)(char *path, char *name, list store);

#ifdef __MINGW32__
static int store_op(int global, list store, storeop_t op) {
	char path[256],name[32];

	if (global) {
		strcpy(path,GLOBAL_DIR);
		strcpy(name,"global");
	} else {
		TCHAR szHomeDirBuf[MAX_PATH] = { 0 };
		DWORD BufSize = MAX_PATH;
		HANDLE hToken = 0;
		long name_len;

		/* Get home dir */
		GetUserName(name, &name_len);
		name[name_len] = 0;
		OpenProcessToken( GetCurrentProcess(), TOKEN_QUERY, &hToken );
		GetUserProfileDirectory( hToken, szHomeDirBuf, &BufSize );
		CloseHandle( hToken );
		strcpy(path,szHomeDirBuf);
	}
	strcat(path,"\\");
	strcat(path,"vim.dat");
	dprintf("path: %s\n", path);

	return op(path,name,store);
}
#else
static int store_op(int global, list store, storeop_t op) {
	char path[256],name[32];

	if (global) {
		strcpy(path,GLOBAL_DIR);
		strcpy(name,"global");
	} else {
		struct passwd *pw;
		int uid;

		/* Get the uid of this process */
		uid = getuid();
		dprintf("uid: %d\n", uid);

		/* Get the password entry for this uid */
		pw = getpwuid(uid);
		if (!pw) {
			perror("vim_get_pass: getpwuid");
			return 1;
		}

		strcpy(name,pw->pw_name);
		strcpy(path,pw->pw_dir);
	}
	strcat(path,"/");
	strcat(path,".vim");
	dprintf("path: %s\n", path);

	return op(path,name,store);
}
#endif

static int store_reader(char *path, char *name, list store) {
	unsigned char key[1024], decoded_pass[32];
	char line[128], encoded_pass[64];
	struct store_rec new_rec;
	rc4_context_t ctx;
	int len;
	FILE *fp;

	dprintf("path: %s, name: %s\n", path, name);

	fp = fopen(path,"r");
	if (!fp) return 1;
	while(fgets(line,sizeof(line),fp)) {
		/* Read a line */
		if (line[0] == '#') continue;
		line[strlen(line)-1] = 0;
		dprintf("line: %s\n", line);

		/* Parse line */
		memset(&new_rec,0,sizeof(new_rec));
		sscanf(line,"%s %s %s", new_rec.server, new_rec.user, encoded_pass);
		dprintf("file: server: %s, user: %s, pass: %s\n", new_rec.server, new_rec.user, encoded_pass);

		/* Decode */
		len = decode(decoded_pass,encoded_pass);

		/* Init key */
		sprintf((char *)key,"%s%s%s", name, new_rec.server, new_rec.user);
		dprintf("key: %s\n", (char *)key);
		rc4_init(&ctx,key,strlen((char *)key));

		/* Decrypt */
		rc4_decrypt(&ctx,new_rec.pass,decoded_pass,len);
		dprintf("pass: %s\n", new_rec.pass);

		/* Add to list */
		list_add(store,&new_rec,sizeof(new_rec));
	}
	fclose(fp);
	return 0;
}
#define store_read(g,l) store_op(g, l, store_reader)

static int store_writer(char *path, char *name, list store) {
	unsigned char key[1024], encrypted_pass[64];
	char encoded_pass[64];
	struct store_rec *rec;
	rc4_context_t ctx;
	int len;
	FILE *fp;

	dprintf("path: %s, name: %s\n", path, name);
	fp = fopen(path,"w+");
	if (!fp) return 1;
	list_reset(store);
	while((rec = list_get_next(store)) != 0) {
		/* Init key */
		sprintf((char *)key,"%s%s%s", name, rec->server, rec->user);
		dprintf("key: %s\n", (char *)key);
		rc4_init(&ctx,key,strlen((char *)key));

		/* Encrypt */
		dprintf("pass: %s\n", rec->pass);
		len = strlen(rec->pass)+1;
		rc4_encrypt(&ctx,encrypted_pass,(unsigned char *)rec->pass,len);

		/* Encode */
		encode(encoded_pass, encrypted_pass, len);
		dprintf("encoded_pass: %s\n", encoded_pass);

		/* Write */
		fprintf(fp, "%s %s %s\n", rec->server, rec->user, encoded_pass);
	}
	fclose(fp);

	return 0;
}
#define store_write(g,l) store_op(g, l, store_writer)

int vim_cred_add(int global, char *server, char *user, char *pass) {
	list store;
	struct store_rec *rec;
	int r,found,updated;
	int x;

	dprintf("server: %s, user: %s, pass: %s\n", server, user, pass);

	store = list_create();

	/* unable to read the store is not an error... */
	store_read(global,store);

	r = 1;

	found = updated = 0;
	list_reset(store);
	while((rec = list_get_next(store)) != 0) {
		dprintf("rec: server: %s, user: %s\n", rec->server, rec->user);
		if (strcmp(rec->server,server) == 0 && strcmp(rec->user,user) == 0) {
//		x = (match(server,rec->server) && strcmp(user,rec->user) == 0);
//		dprintf("x: %d\n", x);
//		if (x) {
			dprintf("old: %s, new: %s\n", rec->pass, pass);
			if (strcmp(rec->pass,pass) != 0) {
				dprintf("updating...\n");
				strcpy(rec->pass,pass);
				updated = 1;
			}
			found = 1;
			break;
		}
	}
	if (!found) {
		struct store_rec new_rec;

		memset(&new_rec,0,sizeof(new_rec));
		strcpy(new_rec.server,server);
		strcpy(new_rec.user,user);
		strcpy(new_rec.pass,pass);

		list_add(store,&new_rec,sizeof(new_rec));
		updated = 1;
	}
	if (updated) {
		if (store_write(global,store)) {
			perror("error writing to store");
			goto done;
		}
	}

	r = 0;

done:
	list_destroy(store);
	return r;
}

int vim_cred_del(int global, char *server, char *user) {
	list store;
	struct store_rec *rec;
	int found,x;

	store = list_create();

	found = 0;
	if (store_read(global,store)) goto done;

	list_reset(store);
	while((rec = list_get_next(store)) != 0) {
//		if (strcmp(rec->server,server) == 0 && strcmp(rec->user,user) == 0) {
		x = (match(server,rec->server) && strcmp(user,rec->user) == 0);
		dprintf("x: %d\n", x);
		if (x) {
			list_delete(store,rec);
			if (store_write(global,store)) goto done;
			found = 1;
			break;
		}
	}

done:
	list_destroy(store);
	return (found ? 0 : 1);
}

char *vim_cred_get(char *server, char *user) {
	list store;
	struct store_rec *rec;
	static char pass[sizeof(rec->pass)];
	int found;

	store = list_create();

	pass[0] = 0;
	/* Get local first */
	dprintf("reading local...\n");
	store_read(0,store);
	/* Get global next */
	dprintf("reading global...\n");
	store_read(1,store);

	dprintf("server: %s, user: %s\n", server, user);
	list_reset(store);
	while((rec = list_get_next(store)) != 0) {
		dprintf("rec: server: %s, user: %s, pass: %s\n", rec->server, rec->user, rec->pass);
		found = (match(server,rec->server) && strcmp(user,rec->user) == 0);
		dprintf("found: %d\n", found);
		if (found) {
			strncat(pass,rec->pass,sizeof(pass)-1);
			break;
		}
	}

//done:
	list_destroy(store);
	dprintf("returning: %s\n", pass);
	return pass;
}
