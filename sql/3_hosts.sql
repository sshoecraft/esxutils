DROP TABLE IF EXISTS hosts;
CREATE TABLE hosts (
	id		INT NOT NULL AUTO_INCREMENT,
	farm_id		INT,
	name		VARCHAR(64) NOT NULL,
	uuid		VARCHAR(64),
	os		VARCHAR(16),
	version		VARCHAR(8),
	build		INT,
	model		VARCHAR(32),
	serial		VARCHAR(32),
	bios		VARCHAR(32),
	status		VARCHAR(16),
	in_maint	VARCHAR(8),
	psp		FLOAT,
	cpu_pkgs	INT,
	cpu_count	INT,
	cpu_model	VARCHAR(64),
	cpu_total	INT,
	cpu_speed	INT,
	mem_total	INT,
	cons_mem	INT,
	subnet		VARCHAR(16),
	last_seen	TIMESTAMP,
	state		VARCHAR(16),
	PRIMARY KEY(id,name)
) ENGINE = InnoDB;
CREATE INDEX hosts_named ON hosts(name);
