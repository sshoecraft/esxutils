
#include "collect.h"
#include "gendb_host_datastore.h"

/* Create host <-> datastore linkage in db */
int update_host_datastores(struct csession *s) {
	struct host_datastore_record rec;
	struct datastore *ds;
	struct host *host;
	char clause[64];

	list_reset(s->hosts);
	while((host = list_get_next(s->hosts)) != 0) {
		list_reset(host->datastores);
		while((ds = list_get_next(host->datastores)) != 0) {
//			printf("ds: id: %d, uuid: %s\n", ds->id, ds->uuid);
			sprintf(clause," WHERE host_id = %d AND datastore_id = %d", host->id,ds->id);
			if (host_datastore_select_record(s->db,&rec,clause)) {
				rec.host_id = host->id;
				rec.datastore_id = ds->id;
				if (host_datastore_insert(s->db,&rec))
					return 1;
			}
		}
	}

	return 0;
}
