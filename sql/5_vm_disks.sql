DROP TABLE IF EXISTS vm_disks;
CREATE TABLE vm_disks (
	vm_id		INT,
	name		varchar(64),
	filename	varchar(256),
	size		BIGINT,
	isthin		varchar(16),
	PRIMARY KEY (vm_id,name)
) ENGINE = InnoDB;
