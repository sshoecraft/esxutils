DROP TABLE IF EXISTS host_net;
CREATE TABLE host_net (
	host_id		INT,
	interface	varchar(64),
	ip		varchar(20),
	subnet		varchar(20),
	PRIMARY KEY (host_id)
) ENGINE = InnoDB;
