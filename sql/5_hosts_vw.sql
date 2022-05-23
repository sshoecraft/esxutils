DROP VIEW IF EXISTS hosts_vw;
create view hosts_vw as select hosts.id, farms.name as farm, hosts.name, hosts.model, hosts.bios, hosts.status, hosts.version, hosts.build, hosts.psp, hosts.cpu_count, hosts.cpu_total, hosts.mem_total, hosts.last_seen from hosts join farms on (farms.id = hosts.farm_id);
