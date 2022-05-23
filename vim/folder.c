
#include "ivim.h"
#include "ServiceContent.h"
#include "CreateDatacenter.h"
#include "CreateCluster.h"

struct ManagedObjectReference *vim_createdc(struct vim_session *vim, char *name) {
	struct CreateDatacenterRequest req;
	struct ManagedObjectReference *dc;
	int r;

	dprintf("name: %s\n", name);

	req.folder = vim->sc->rootFolder;
	req.name = name;
	r = CreateDatacenter(vim->soap, vim->endpoint, &req, &dc);
	dprintf("r: %d\n", r);
	if (r) return 0;
	return dc;
}

struct ManagedObjectReference *vim_createcluster(struct vim_session *vim, struct ManagedObjectReference *folder, char *name, struct ClusterConfigSpec *spec) {
	struct CreateClusterRequest req;
	struct ManagedObjectReference *obj;
	int r;

	dprintf("name: %s\n", name);

	memset(&req,0,sizeof(req));
	req.folder = folder;
	req.name = name;
	req.spec = spec;
	r = CreateCluster(vim->soap, vim->endpoint, &req, &obj);
	dprintf("r: %d\n", r);
	if (r) return 0;
	return obj;
}
