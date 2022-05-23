DROP TABLE IF EXISTS vm_perf;
CREATE TABLE vm_perf (
	time		TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
	vm_id		INT,
	cpu		INT,
	ready		INT,
	mem		INT,
	disk		INT,
	net		INT,
	PRIMARY KEY (time,vm_id)
) ENGINE = InnoDB;
