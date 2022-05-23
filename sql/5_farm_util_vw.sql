drop view IF EXISTS farm_util_vw;
create view farm_util_vw AS select time, farms.name AS farm,round(avg(((cpu / hosts.cpu_total) * 100)),0) AS cpu_util,round(avg(((mem / hosts.mem_total) * 100)),0) AS mem_util, round(avg(disk),0) as disk_rate, round(avg(net),0) AS net_rate from host_perf join hosts on (host_perf.host_id = hosts.id) join farms on (hosts.farm_id = farms.id);
