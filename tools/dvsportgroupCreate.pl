#!/usr/bin/perl
# dvsportgroupCreate.pl
# Create a distributed virtual portgroup (vlan id type) on a specified dvs
# To see command options, run first with '--help'
# by www.lincvz.info
# v1.0

use strict;
use warnings;
use VMware::VIRuntime;

my $dvpgtype = 'earlyBinding';
my $numports = 128;

######################################
# Define options
######################################
my %opts = (
      dvs => {
      type => "=s",
      help => "Distributed Virtual Switch name",
      required => 1,
      },

      portgroup => {
      type => "=s",
      help => "Distributed Virtual Port Group name",
      required => 1,
      },

      vlan => {
      type => "=i",
      help => "Distributed Virtual Port Group VLAN ID",
      required => 1,
      },
);

Opts::add_options(%opts);
Opts::parse();
Opts::validate();

Util::connect();

######################################
# Check if DVS exist
######################################
my $dvs_name = Opts::get_option('dvs');
my $dvs_view = Vim::find_entity_view(
      view_type => 'DistributedVirtualSwitch',
      filter => { 'name' => $dvs_name },
      );
if (not defined $dvs_view) {
      Util::trace(0, "ERR: Distributed Virtual Switch $dvs_name does not exist\n");
      exit(1);
}

######################################
# Create dvsPortGroup
######################################
my $dvportgroup_name = Opts::get_option('portgroup');
my $vlanid = Opts::get_option('vlan');
my $dvs_vlan_spec = VmwareDistributedVirtualSwitchVlanIdSpec->new(
                                                              vlanId => $vlanid,
                                                              inherited => '0');
my $port_settings = VMwareDVSPortSetting->new(vlan => $dvs_vlan_spec);
my $dvpg_configspec = DVPortgroupConfigSpec->new(
      name => $dvportgroup_name,
      defaultPortConfig => $port_settings,
      type => $dvpgtype,
      numPorts => $numports,
      );

######################################
# Call CreateDVPortgroup task
######################################
eval { $dvs_view->CreateDVPortgroup(spec => $dvpg_configspec); };

######################################
# Check faults
######################################
if ($@) {
      if (ref($@) eq 'SoapFault') {
            if (ref($@->detail) eq 'DuplicateName') {
                  Util::trace(0, "ERR: a portgroup with the same name already exists.\n");
            }
            elsif (ref($@->detail) eq 'DvsFault') {
                  Util::trace(0, "ERR: operation fails on any host.\n");
            }
            elsif (ref($@->detail) eq 'InvalidName') {
                  Util::trace(0, "ERR: name of the portgroup is invalid.\n");
            }
            elsif (ref($@->detail) eq 'RuntimeFault') {
                  Util::trace(0, "ERR: runtime fault.\n");
            }
            else {
                  Util::trace(0, "ERR: ". $@ . "\n");
            }
      }
      else {
           Util::trace(0, "ERR: ". $@ . "\n");
      }
      exit(1);
}
