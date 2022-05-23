drop view IF EXISTS vm_perf_vw;
create view vm_perf_vw AS select vm_perf.time AS time, vms.name AS vm, cpu,ready,mem,disk,net from vm_perf join vms on (vm_perf.vm_id = vms.id);
