DROP VIEW IF EXISTS alarms_vw;
create view alarms_vw as select farms.name as farm, hosts.name as host, substr(alarms.alarm,1,64), alarms.status from host_alarms as alarms,hosts,farms where alarms.host_id = hosts.id and hosts.farm_id = farms.id;
