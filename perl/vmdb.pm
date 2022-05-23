package vmdb;
use utils;
use strict;
use warnings;

use DBI;
use Data::Dumper;

use lib qw (/usr/local/lib/perl);
use utils;

my $DB_DRIVER = 'odbc';
my $DB_NAME = 'esxadmin';
my $DB_USER = 'esxadmin_rw';
my $DB_PASS = 'esxadmin_rw';

my $vmdb = DBI->connect('DBI:mysql:'.$DB_NAME,$DB_USER,$DB_PASS,{RaiseError => 1}) or die "Couldn't connect to VMDB: " . DBI->errstr;

#my $farms = $vmdb->prepare("SELECT id,name FROM farms");
my $farms = $vmdb->prepare("SELECT * FROM farms");
my $farm_vms = $vmdb->prepare("SELECT * FROM vms WHERE farm_id = ?");
#my $hosts = $vmdb->prepare("SELECT id,name,os,version,build,model,serial FROM hosts WHERE farm_id = ?");
my $hosts = $vmdb->prepare("SELECT * FROM hosts WHERE farm_id = ?");
my $host_vms = $vmdb->prepare("SELECT * FROM vms WHERE host_id = ?");

my ($farms_newrec,$farms_select_id,$farms_insert_nm,$farms_select,$farms_insert,$farms_update);
my ($hosts_newrec,$hosts_select_id,$hosts_insert_nm,$hosts_select,$hosts_insert,$hosts_update);
my ($vms_newrec,$vms_select_id,$vms_insert_nm,$vms_select,$vms_insert,$vms_update);

sub disconnect {
	$vmdb->commit;
	$vmdb->disconnect;
	$vmdb = undef;
}

sub _do_prepare($$) {
	unless ($vmdb) {
		dprintf(3,"_do_prepare: connecting...\n");
#		my ($db_pass) = utils::get_cred($DB_NAME,$DB_USER);
		my $db_pass = $DB_PASS;
		$vmdb = DBI->connect($DB_DRIVER . $DB_NAME,$DB_USER,$db_pass,{RaiseError => 1})
	}
	unless($_[0]) {
		dprintf(3,"_do_prepare: preparing: %s\n", $_[1]);
		$_[0] = $vmdb->prepare($_[1]);
	}
}

sub farms_newrec() {
	_do_prepare($farms_newrec,"select * from (select 1 as _remove_this) a left join (  select * from farms where id = NULL  ) t on 1=1;");
	$farms_newrec->execute();
	my $rec = $farms_newrec->fetchrow_hashref();
	delete $rec->{_remove_this};
	return $rec;
}

sub farms_select_id($$) {
	my ($doins,$name) = @_;
	my $id;
	dprintf(3,"farms_select_id: name: %s\n", $name);
	_do_prepare($farms_select_id,"SELECT id FROM farms WHERE name = ?");
	$farms_select_id->execute($name);
	($id) = $farms_select_id->fetchrow_array();
	if (!defined($id) && $doins == 1) {
		_do_prepare($farms_insert_nm,"INSERT INTO farms (name) VALUES (?)");
		$farms_insert_nm->execute($name);
		$farms_select_id->execute($name);
		($id) = $farms_select_id->fetchrow_array();
	}
	dprintf(3,"farms_select_id: id: %s\n", $id);
	return $id;
}

sub farms_select($) {
	dprintf(3,"farms_select: id: %s\n", $_[0]);
	_do_prepare($farms_select,"SELECT * FROM farms WHERE id = ?");
	$farms_select->execute($_[0]);
	return $farms_select->fetchrow_hashref();
}

sub farms_insert($) {
	delete $_[0]->{id};
	_do_prepare($farms_insert,"INSERT INTO farms (" . (join ",", keys %{$_[0]}) . ") VALUES (" . (join ", ", map {'?'} keys %{$_[0]}) . ")");
	$farms_insert->execute(values %{$_[0]});
	$_[0]->{id} = farms_select_id(0,$_[0]->{name});
	dprintf(3,"farms_insert: id: %s\n", $_[0]->{id});
	return $_[0]->{id};
}

sub farms_update($) {
	# pull the key out of the rec then remove from rec
	my $id = $_[0]->{id};
	dprintf(3,"farms_update: id: %d\n", $id);
	delete $_[0]->{id};

	# add the key to the end of the values
	my @values = values %{$_[0]};
	push @values,$id;

	unless($farms_update) {
		my $query = "UPDATE farms SET ";
		my $num = 0;
		foreach my $col (keys %{$_[0]}) {
			$query .= "," if ($num++);
			$query .= $col . " = ?"
		}
		$query .= " WHERE id = ?";
		dprintf(3,"farms_update: query: %s\n", $query);
		_do_prepare($farms_update,$query);
	}
	$_[0]->{id} = $id;
#	print Dumper(@values);
	my $status = $farms_update->execute(@values);
	$status = -1 unless($status);
	dprintf(3,"farms_update: status: %s\n", $status);
	return ($status == 1 ? 0 : 1);
}

sub hosts_newrec() {
	_do_prepare($hosts_newrec,"select * from (select 1 as _remove_this) a left join (  select * from hosts where id = NULL  ) t on 1=1;");
	$hosts_newrec->execute();
	my $rec = $hosts_newrec->fetchrow_hashref();
	delete $rec->{_remove_this};
	return $rec;
}

sub hosts_select_id($$) {
	my ($doins,$name) = @_;
	my $id;
	dprintf(3,"hosts_select_id: name: %s\n", $name);
	_do_prepare($hosts_select_id,"SELECT id FROM hosts WHERE name = ?");
	$hosts_select_id->execute($name);
	($id) = $hosts_select_id->fetchrow_array();
	if (!defined($id) && $doins == 1) {
		_do_prepare($hosts_insert_nm,"INSERT INTO hosts (name) VALUES (?)");
		$hosts_insert_nm->execute($name);
		$hosts_select_id->execute($name);
		($id) = $hosts_select_id->fetchrow_array();
	}
	dprintf(3,"hosts_select_id: id: %s\n", $id);
	return $id;
}

sub hosts_select($) {
	dprintf(3,"hosts_select: id: %s\n", $_[0]);
	_do_prepare($hosts_select,"SELECT * FROM hosts WHERE id = ?");
	$hosts_select->execute($_[0]);
	return $hosts_select->fetchrow_hashref();
}

sub hosts_insert($) {
	delete $_[0]->{id};
	_do_prepare($hosts_insert,"INSERT INTO hosts (" . (join ",", keys %{$_[0]}) . ") VALUES (" . (join ", ", map {'?'} keys %{$_[0]}) . ")");
	$hosts_insert->execute(values %{$_[0]});
	$_[0]->{id} = hosts_select_id(0,$_[0]->{name});
	dprintf(3,"hosts_insert: id: %s\n", $_[0]->{id});
	return $_[0]->{id};
}

sub hosts_update($) {
	# pull the key out of the rec then remove from rec
	my $id = $_[0]->{id};
	dprintf(3,"hosts_update: id: %d\n", $id);
	delete $_[0]->{id};

	# add the key to the end of the values
	my @values = values %{$_[0]};
	push @values,$id;

	unless($hosts_update) {
		my $query = "UPDATE hosts SET ";
		my $num = 0;
		foreach my $col (keys %{$_[0]}) {
			$query .= "," if ($num++);
			$query .= $col . " = ?"
		}
		$query .= " WHERE id = ?";
		dprintf(3,"hosts_update: query: %s\n", $query);
		_do_prepare($hosts_update,$query);
	}
	$_[0]->{id} = $id;
#	print Dumper(@values);
	my $status = $hosts_update->execute(@values);
	$status = -1 unless($status);
	dprintf(3,"hosts_update: status: %s\n", $status);
	return ($status == 1 ? 0 : 1);
}

sub vms_newrec() {
	_do_prepare($vms_newrec,"select * from (select 1 as _remove_this) a left join (  select * from vms where id = NULL  ) t on 1=1;");
	$vms_newrec->execute();
	my $rec = $vms_newrec->fetchrow_hashref();
	delete $rec->{_remove_this};
	return $rec;
}

sub vms_select_id($$) {
	my ($doins,$name) = @_;
	my $id;
	dprintf(3,"vms_select_id: name: %s\n", $name);
	_do_prepare($vms_select_id,"SELECT id FROM vms WHERE name = ?");
	$vms_select_id->execute($name);
	($id) = $vms_select_id->fetchrow_array();
	if (!defined($id) && $doins == 1) {
		_do_prepare($vms_insert_nm,"INSERT INTO vms (name) VALUES (?)");
		$vms_insert_nm->execute($name);
		$vms_select_id->execute($name);
		($id) = $vms_select_id->fetchrow_array();
	}
	dprintf(3,"vms_select_id: id: %s\n", $id);
	return $id;
}

sub vms_select($) {
	dprintf(3,"vms_select: id: %s\n", $_[0]);
	_do_prepare($vms_select,"SELECT * FROM vms WHERE id = ?");
	$vms_select->execute($_[0]);
	return $vms_select->fetchrow_hashref();
}

sub vms_insert($) {
	delete $_[0]->{id};
	_do_prepare($vms_insert,"INSERT INTO vms (" . (join ",", keys %{$_[0]}) . ") VALUES (" . (join ", ", map {'?'} keys %{$_[0]}) . ")");
	$vms_insert->execute(values %{$_[0]});
	$_[0]->{id} = vms_select_id(0,$_[0]->{name});
	dprintf(3,"vms_insert: id: %s\n", $_[0]->{id});
	return $_[0]->{id};
}

sub vms_update($) {
	# pull the key out of the rec then remove from rec
	my $id = $_[0]->{id};
	dprintf(3,"vms_update: id: %d\n", $id);
	delete $_[0]->{id};

	# add the key to the end of the values
	my @values = values %{$_[0]};
	push @values,$id;

	unless($vms_update) {
		my $query = "UPDATE vms SET ";
		my $num = 0;
		foreach my $col (keys %{$_[0]}) {
			$query .= "," if ($num++);
			$query .= $col . " = ?"
		}
		$query .= " WHERE id = ?";
		dprintf(3,"vms_update: query: %s\n", $query);
		_do_prepare($vms_update,$query);
	}
	$_[0]->{id} = $id;
#	print Dumper(@values);
	my $status = $vms_update->execute(@values);
	$status = -1 unless($status);
	dprintf(3,"vms_update: status: %s\n", $status);
	return ($status == 1 ? 0 : 1);
}

1
