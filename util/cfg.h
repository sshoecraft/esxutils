#ifndef __CFG_H
#define __CFG_H

/*************************************************************************
 *
 * Configuration file functions
 *
 *************************************************************************/

#include "list.h"

/* Define the item structure */
#define CFG_KEYWORD_SIZE	64
struct _cfg_item {
	char keyword[CFG_KEYWORD_SIZE];
	char *desc;
	char *value;
};
typedef struct _cfg_item CFG_ITEM;
#define CFG_ITEM_SIZE sizeof(struct _cfg_item)

/* Define the section info structure */
#define CFG_SECTION_NAME_SIZE	64
struct _cfg_section {
	char name[CFG_SECTION_NAME_SIZE];
	list items;
};
typedef struct _cfg_section CFG_SECTION;
#define CFG_SECTION_SIZE sizeof(struct _cfg_section)

/* Define the cfg info structure */
struct _cfg_info {
	char filename[256];	/* Filename */
	list sections;		/* Section list */
};
typedef struct _cfg_info CFG_INFO;
#define CFG_INFO_SIZE sizeof(struct _cfg_info)

struct cfg_proctab {
	char *section;
	char *keyword;
	char *desc;
	int type;
	void *dest;
	int dlen;
	void *def;
};

#define CFG_WRITE_ONLY 0			/* Write only non-null strings */
#define CFG_WRITE_NULL 1			/* Write null string values (as empty) */

#define CFG_PROCTAB_END { 0, 0, 0, 0, 0, 0 }

#ifdef __cplusplus
extern "C" {
#endif
/* functions */
CFG_INFO *cfg_create(char *);
CFG_SECTION *cfg_create_section(CFG_INFO *,char *);
void cfg_destroy(CFG_INFO *);
CFG_INFO *cfg_read(char *);
int cfg_write(CFG_INFO *);
CFG_SECTION *cfg_get_section(CFG_INFO *,char *);

char *cfg_get_item(CFG_INFO *,char *,char *);
char *cfg_get_string(CFG_INFO *,char *,char *,char *);
int cfg_get_bool(CFG_INFO *, char *, char *, int);
int cfg_get_int(CFG_INFO *, char *, char *, int);
long long cfg_get_quad(CFG_INFO *, char *, char *, long long);
double cfg_get_double(CFG_INFO *, char *, char *, double);
list cfg_get_list(CFG_INFO *, char *, char *, char *);

int cfg_set_item(CFG_INFO *, char *, char *, char *, char *);
#define cfg_set_string cfg_set_item
int cfg_set_bool(CFG_INFO *, char *, char *, int);
int cfg_set_int(CFG_INFO *, char *, char *, int);
int cfg_set_quad(CFG_INFO *, char *, char *, long long);
int cfg_set_double(CFG_INFO *, char *, char *, double);
int cfg_set_list(CFG_INFO *, char *, char *, list);

int cfg_get_tab(CFG_INFO *,struct cfg_proctab *);
int cfg_set_tab(CFG_INFO *,struct cfg_proctab *,int);
void cfg_disp_tab(struct cfg_proctab *,char *,int);
struct cfg_proctab *cfg_combine_tabs(struct cfg_proctab *,struct cfg_proctab *);

#ifdef __cplusplus
}
#endif

#endif /* __CFG_H */
