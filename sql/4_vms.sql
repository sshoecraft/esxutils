DROP TABLE IF EXISTS vms;
CREATE TABLE vms (
	id		INT NOT NULL AUTO_INCREMENT,
	name		VARCHAR(128),
	farm_id		INT,
	host_id		INT,
	ci_name		VARCHAR(256),
	uuid		VARCHAR(64),
	os		VARCHAR(64),
	tools		VARCHAR(16),
	state		VARCHAR(32),
	network		VARCHAR(128),
	ip		VARCHAR(16),
	mac		VARCHAR(24),
	cpu_total	INT,
	cpu_usage	INT,
	mem_total	INT,
	mem_usage	INT,
	size_total	INT,
	boot_time	TIMESTAMP,
	annotation	VARCHAR(1024),
	last_seen	TIMESTAMP,
	PRIMARY KEY (id,farm_id,name)
) ENGINE = InnoDB;
