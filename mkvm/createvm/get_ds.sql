select * from datastores where id in (select datastore_id from host_datastore where host_id in (select id from hosts where farm_id in (select id from farms where name = 'Misc12')));
