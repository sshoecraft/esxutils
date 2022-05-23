DROP TABLE IF EXISTS farms;
CREATE TABLE farms (
	id		INT UNIQUE AUTO_INCREMENT,
	site_id		INT,
	server		VARCHAR(64),
	name		VARCHAR(64) NOT NULL,
	last_seen	TIMESTAMP,
	PRIMARY KEY (id,server,name)
) ENGINE = InnoDB;
CREATE INDEX farms_server ON farms(server);
CREATE INDEX farms_named ON farms(name);
