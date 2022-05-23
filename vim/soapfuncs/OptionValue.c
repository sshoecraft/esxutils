
#include "OptionValue.h"
#include "list.h"

int put_OptionValue(struct soap *soap, char *tag, void **ptr, int req) {
	list opts = (list) *ptr;
	if (opts) {
		struct OptionValue realopt,*opt;
		struct mydesc put_opt_desc[] = {
			{ "key", &realopt.key, put_char, 0 },
			{ "value", &realopt.value, put_char, 0 },
			{ 0,0,0,0 }
		};

		list_reset(opts);
		while((opt = list_get_next(opts)) != 0) {
			realopt.key = opt->key;
			realopt.value = opt->value;
			if (soap_send_desc(soap,tag,"xsi:type","OptionValue",put_opt_desc))
				return 1;
		}
	}
	return 0;
}
