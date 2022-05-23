DROP TABLE IF EXISTS esx_build_name;
CREATE TABLE esx_build_name (
	build		INT,
	name		VARCHAR(32),
	PRIMARY KEY (build)
) ENGINE = InnoDB;
INSERT INTO esx_build_name VALUES (0,'Unknown');
