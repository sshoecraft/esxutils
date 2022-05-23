DROP TABLE IF EXISTS host_datastore;
CREATE TABLE host_datastore (
	host_id		INT,
	datastore_id	INT,
	PRIMARY KEY (host_id,datastore_id)
) ENGINE = InnoDB;
