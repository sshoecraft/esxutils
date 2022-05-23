DROP TABLE IF EXISTS vm_files;
CREATE TABLE vm_files (
	vm_id		INT,
	name		varchar(256),
	type		varchar(64),
	size		BIGINT,
	PRIMARY KEY (vm_id,name)
) ENGINE = InnoDB;
