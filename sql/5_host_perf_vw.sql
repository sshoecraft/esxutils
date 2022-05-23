drop view IF EXISTS host_perf_vw;
create view host_perf_vw AS select host_perf.time AS time, hosts.name AS host, cpu,mem,disk,net from host_perf join hosts on (host_perf.host_id = hosts.id);
