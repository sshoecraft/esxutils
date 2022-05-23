DROP TABLE IF EXISTS datastores;
CREATE TABLE datastores (
	id		INT NOT NULL AUTO_INCREMENT,
	uuid		VARCHAR(128) NOT NULL UNIQUE,
	name		VARCHAR(128),
	blocksize	INT,
	total		INT,
	free		INT,
	last_seen	TIMESTAMP,
	PRIMARY KEY (id)
) ENGINE = InnoDB;
CREATE INDEX datastores_uuid_idx on datastores (uuid);
