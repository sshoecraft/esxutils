DROP TABLE IF EXISTS upfarm;
CREATE TABLE upfarm (
	name		VARCHAR(64),
	status		INT,
	PRIMARY KEY (name)
);

DROP TABLE IF EXISTS uphost;
CREATE TABLE uphost (
	name		VARCHAR(64),
	status		INT,
	PRIMARY KEY (name)
);
