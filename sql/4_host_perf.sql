DROP TABLE IF EXISTS host_perf;
CREATE TABLE host_perf (
	time		TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	host_id		INT,
	cpu		INT,
	mem		INT,
	disk		INT,
	net		INT,
	PRIMARY KEY (time,host_id)
) ENGINE = InnoDB;
