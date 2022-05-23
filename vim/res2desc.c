
#ifdef DEBUG
#undef DEBUG
#endif
//#define DEBUG		1

#include "ivim.h"
#include "RetrieveProperties.h"

int vim_results2desc(struct vim_session *vim, list results, struct vim_res2desc *desc) {
	struct vim_res2desc *p;
	char *val;
	char *pstr = 0;
	char **ppstr = 0;
	short *pshort = 0;
	int *pint = 0;
	long long *pquad = 0;
	float *pfloat = 0;
	double *pdub = 0;
	bool *pbool = 0;
	struct ManagedObjectReference *pobj = 0;
	struct ManagedObjectReference **ppobj = 0;

	for(p = desc; p->path; p++) {
		dprintf("path: %s, type: %d, dest: %p\n", p->path, p->type, p->dest);
		if (!p->dest) continue;
		switch(p->type) {
		case VIM_TYPE_OBJ:
			pobj = p->dest;
			break;
		case VIM_TYPE_POBJ:
			ppobj = p->dest;
			break;
		case VIM_TYPE_PSTRING:
			ppstr = p->dest;
			break;
		case VIM_TYPE_STRING:
		case VIM_TYPE_BYTE:
			pstr = p->dest;
			pstr[0] = 0;
			break;
		case VIM_TYPE_SHORT:
			pshort = p->dest;
			break;
		case VIM_TYPE_INT:
			pint = p->dest;
			break;
		case VIM_TYPE_QUAD:
			pquad = p->dest;
			break;
		case VIM_TYPE_FLOAT:
			pfloat = p->dest;
			break;
		case VIM_TYPE_DOUBLE:
			pdub = p->dest;
			break;
		case VIM_TYPE_BOOL:
			pbool = p->dest;
			break;
		default:
			dprintf("unknown type: %d\n", p->type);
			continue;
			break;
		}
		val = get_result_value(results,p->path);
		dprintf("val: %s\n", val);
		/* Default val */
		if (val && strlen(val) == 0) val = 0;
		if (!val) {
			if (p->def) {
				switch(p->type) {
				case VIM_TYPE_OBJ:
					pobj = (struct ManagedObjectReference *) p->def;
					break;
				case VIM_TYPE_POBJ:
					*ppobj = (struct ManagedObjectReference *) p->def;
					break;
				case VIM_TYPE_STRING:
					strncat(pstr, p->def, p->dlen);
					break;
				case VIM_TYPE_PSTRING:
					*ppstr = p->def;
					break;
				case VIM_TYPE_BYTE:
					*pstr = *p->def;
					break;
				case VIM_TYPE_SHORT:
					*pshort = atoi(p->def);
					break;
				case VIM_TYPE_INT:
					*pint = atoi(p->def);
					break;
				case VIM_TYPE_QUAD:
					*pquad = strtoll(p->def, 0, 10);
					break;
				case VIM_TYPE_FLOAT:
					*pfloat = atof(p->def);
					break;
				case VIM_TYPE_DOUBLE:
					*pdub = strtod(p->def, 0);
					break;
				case VIM_TYPE_BOOL:
					*pbool = (strcmp(p->def,"true") == 0 ? 1 : 0);
					break;
				}
			}
		} else {
			switch(p->type) {
			case VIM_TYPE_OBJ:
				{
					struct propSet *set;
					set = get_result(results,p->path);
					dprintf("set: type: %s, value: %s\n", set->type, set->value);
					pobj->type = set->type;
					pobj->value = set->value;
				}
				break;
			case VIM_TYPE_POBJ:
				{
					struct propSet *set;
					set = get_result(results,p->path);
					dprintf("set: type: %s, value: %s\n", set->type, set->value);
					*ppobj = soap_malloc(vim->soap, sizeof(struct ManagedObjectReference));
					(*ppobj)->type = set->type;
					(*ppobj)->value = set->value;
				}
				break;
			case VIM_TYPE_STRING:
				strncat(pstr, val, p->dlen);
				break;
			case VIM_TYPE_PSTRING:
				*ppstr = val;
				break;
			case VIM_TYPE_BYTE:
				*pstr = *val;
				break;
			case VIM_TYPE_SHORT:
				*pshort = atoi(val);
				break;
			case VIM_TYPE_INT:
				*pint = atoi(val);
				break;
			case VIM_TYPE_QUAD:
				*pquad = strtoll(val, 0, 10);
				break;
			case VIM_TYPE_FLOAT:
				*pfloat = atof(val);
				break;
			case VIM_TYPE_DOUBLE:
				*pdub = strtod(val, 0);
				break;
			case VIM_TYPE_BOOL:
				*pbool = (strcmp(val,"true") == 0 ? 1 : 0);
				break;
			}
		}
	}
	return 0;
}
