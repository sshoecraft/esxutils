#!/usr/bin/perl -w
 #
 # Requires a text file named VMToolsUpgradeList.txt
 # containing VM names which the last of must not end with a CR
 # Author: Mike La Spina
 #   Date: May 24, 2008
 # upgradetools.pl .host TheVC

use strict;
use warnings;
use VMware::VIRuntime;

my %opts = (
    'vmname' => {
       type => "=s",
       help => "VM name",
       required => 1,
    },
 );

Opts::add_options(%opts);
Opts::parse();
Opts::validate();
my $vmname = Opts::get_option( 'vmname' );
my ($property_name, $property_value);
$property_name = "config.name";
$property_value = "";
Util::connect();

# get all VMs with out of date tools
# filter => { 'guest.toolsStatus' => 'toolsOld' });
my $vm_views = Vim::find_entity_views(view_type => 'VirtualMachine');

foreach (@$vm_views) {
#	printf("name: %s\n", $_->name);
	if ($_->name eq $vmname) {
		printf("mounting...\n");
#		$_->UpgradeTools_Task();
		$_->MountToolsInstaller();
	}
}
Util::disconnect();
