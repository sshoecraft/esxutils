
#include "ivim.h"
#include "Destroy.h"
#include "Rename.h"
#include "PowerOffVM.h"

int vim_rename(struct vim_session *s, struct ManagedObjectReference *mo_ref, char *newname) {
	struct ManagedObjectReference *task;
	struct RenameRequest req;

	dprintf("mo_ref: %p, newName: %s\n", mo_ref, newname);
	req.obj = mo_ref;
	req.newName = newname;
	if (Rename(s->soap,s->endpoint,&req,&task)) return 1;

	/* Wait for task to complete */
	dprintf("task: %p\n", task);
	return vim_wait4task(s,task,0);
}

int vim_destroy(struct vim_session *s, struct ManagedObjectReference *mo_ref) {
	struct ManagedObjectReference *task;

	/* Remove it from VC */
	if (Destroy(s->soap,s->endpoint,mo_ref,&task)) return 1;

	/* Wait for task to complete */
	return vim_wait4task(s,task,0);
}

int vim_poweroff(struct vim_session *s, struct ManagedObjectReference *mo_ref) {
	struct ManagedObjectReference *task;

	/* Remove it from VC */
	if (PowerOffVM(s->soap,s->endpoint,mo_ref,&task)) return 1;

	/* Wait for task to complete */
	return vim_wait4task(s,task,0);
}
