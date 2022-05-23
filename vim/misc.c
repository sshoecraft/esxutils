
#include "ivim.h"
#include "Destroy.h"

int vim_destroy(struct vim_session *s, struct ManagedObjectReference *mo_ref) {
	struct ManagedObjectReference *task;

	/* Remove it from VC */
	if (Destroy(s->soap,s->endpoint,mo_ref,&task))
		return 1;

	/* Wait for task to complete */
	return vim_wait4task(s,task,0);
}
