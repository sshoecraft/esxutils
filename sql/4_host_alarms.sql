DROP TABLE IF EXISTS host_alarms;
CREATE TABLE host_alarms (
	host_id		INT,
	alarm		VARCHAR(512),
	status		VARCHAR(16),
	reported	TINYINT DEFAULT 0,
	PRIMARY KEY (host_id,alarm)
) ENGINE = InnoDB;
