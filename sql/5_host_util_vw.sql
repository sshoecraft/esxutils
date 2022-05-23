drop view IF EXISTS host_util_vw;
create view host_util_vw AS select time, hosts.name AS host,((cpu / hosts.cpu_total) * 100) AS cpu_util,((mem / hosts.mem_total) * 100) AS mem_util, disk as disk_rate, net AS net_rate from host_perf join hosts on (host_perf.host_id = hosts.id);
