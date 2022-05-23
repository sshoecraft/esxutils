#!/usr/bin/perl -w
#
# Copyright 2012 VM Racks.  All rights reserved.

# Date: 20120317
# Name: vmCDtray.pl
# Description: 
# Script for unloading virtual CD by reconnecting CD to "client". 
# This is important to do before putting a host into maintenance mode.
# When going into maintenance mode VMs are automatically migrated to 
# another ESXi host; however, a VM won't migrate if it has CD loaded
# as an ISO image on a datastore, indicated by "Datastore ISO File". 
#
# Usage:
#   vmCDtray.pl --config /home/vi-admin/vcenter.cfg --url=https://escovc.vmracks.com/sdk --datacenter Esco --host 192.168.1.27 
#   vmCDtray.pl --config /home/vi-admin/vcenter.cfg --url=https://escovc.vmracks.com/sdk --datacenter Esco --host 192.168.1.27  --vmname dev2
#   vmCDtray.pl --config /home/vi-admin/vcenter.cfg --url=https://escovc.vmracks.com/sdk --datacenter Esco --host 192.168.1.27  --host america.vmracks.com
#

use strict;
use warnings;

use VMware::VIRuntime;
use Data::Dumper;

my %opts = (
 datacenter => {
      type => "=s",
      help => "Datacenter name",
      required => 1,
   },

 host => {
      type => "=s",
      help => "ESXi host",
      required => 0,
   },
 vmname => {
      type => "=s",
      help => "virtual machine name",
      required => 0,
   },
);

Opts::add_options(%opts);
Opts::parse();
Opts::validate();
Util::connect();

Util::trace( 0,"\nConnection to VC established...\n" );
my $datacenter = Opts::get_option( 'datacenter' );
my $hostName   = Opts::option_is_set( 'host' )   ? Opts::get_option( 'host' )   : '.*';
my $vmName     = Opts::option_is_set( 'vmname' ) ? Opts::get_option( 'vmname' ) : '.*';

my $datacenterView = Vim::find_entity_view(  view_type => 'Datacenter', properties => [ 'name' ], filter => { name => $datacenter } );

die "Datacenter $datacenter isn't viewable!\n" if ! defined $datacenterView;

Util::trace( 0,"\nDatacenter view loaded...\n" );

my $hostViews = Vim::find_entity_views( view_type => 'HostSystem', begin_entity => $datacenterView, filter => { 'name' => qr/^$hostName$/ } );

foreach my $hostView ( @$hostViews )
{
#	print Dumper($hostView->summary->config);
	printf("name: %s\n", $hostView->summary->config->name);
#	next;
  my $vmViews  = Vim::find_entity_views( view_type => 'VirtualMachine', begin_entity => $hostView, filter => { 'name' => qr/^$vmName$/ } );

  die "vmViews isn't viewable!\n" if ! defined $vmViews;

  foreach my $thisVM ( @$vmViews )
  {
#	print Dumper($thisVM->summary->config);
	printf("name: %s\n", $thisVM->summary->config->name);
#	next;
   	my $deviceList = $thisVM->config->hardware->device;
        #print Dumper( $deviceList );
   	# Does the VM have LAN or Backup Network cable?
   	foreach my $device (@$deviceList)
   	{
           if ( defined $device->{deviceInfo} && defined $device->{connectable} )
           {
             next if $device->deviceInfo->label !~ /CD\/DVD/;
             #print Dumper ( $device ); 

             if ( $device->deviceInfo->summary !~ /Remote (ATAPI|Device)/ )
             {
                print $thisVM->name, ' ', $device->deviceInfo->label, ' ', $device->deviceInfo->summary, "\n";
		&changeDevice( $thisVM, $device );
             }
             #print $thisVM->name, ' ', $device->deviceInfo->label, ' ', $device->deviceInfo->summary;
             #print $device->connectable->connected ? "true" : "false", "\n";
           }
        }
  }
}
Util::disconnect();

#--------------------------------------------------------
sub changeDevice {
#--------------------------------------------------------
  my ( $vm_view, $device ) = @_;
  # there is also a VirtualCdromAtapiBackingInfo, but it didn't work
  # blank device name means the CD setting will be set to "Client Device"
  my $backingInfo = VirtualCdromRemoteAtapiBackingInfo->new( deviceName => '' );
  #my $networkSpec = VirtualPCNet32->new( key => $key, backing => $backingInfo );
  my $key = $device->key;
  my $ckey = $device->controllerKey;
  my $cdSpec = VirtualCdrom->new( key => $key, controllerKey => $ckey, backing => $backingInfo );

  my $spec = VirtualMachineConfigSpec->new(
            changeVersion => $vm_view->config->changeVersion,
            deviceChange => [
               VirtualDeviceConfigSpec->new(
                  operation => VirtualDeviceConfigSpecOperation->new( "edit" ),
                  device => $cdSpec,
               )
            ]
  );
  eval {
            $vm_view->ReconfigVM(spec => $spec);
  };
  if ($@) {
            print STDERR "Reason: " . $@->fault_string . "\n";
  }
}

__END__
Below shoes the entries of what the settings look like when a CD is laoded as a "Datastore ISO File" and when 
the CD is mounted as a "Client Device".

"Datastore ISO File" used as CD Image:

$VAR1 = bless( {
                 'connectable' => bless( {
                                           'allowGuestControl' => '1',
                                           'status' => 'untried',
                                           'connected' => '0',
                                           'startConnected' => '0'
                                         }, 'VirtualDeviceConnectInfo' ),
                 'backing' => bless( {
                                       'datastore' => bless( {
                                                               'value' => 'datastore-443',
                                                               'type' => 'Datastore'
                                                             }, 'ManagedObjectReference' ),
                                       'fileName' => '[burma-nas2-isoimages] nexus-1010.4.2.1.SP1.3.iso'
                                     }, 'VirtualCdromIsoBackingInfo' ),
                 'unitNumber' => '0',
                 'deviceInfo' => bless( {
                                          'summary' => 'ISO [burma-nas2-isoimages] nexus-1010.4.2.1.SP1.3.iso',
                                          'label' => 'CD/DVD drive 1'
                                        }, 'Description' ),
                 'controllerKey' => '201',
                 'key' => '3002'
               }, 'VirtualCdrom' );


CD Connected to Client Device (this is what we want)
$VAR1 = bless( {
                 'connectable' => bless( {
                                           'allowGuestControl' => '1',
                                           'status' => 'untried',
                                           'connected' => '0',
                                           'startConnected' => '0'
                                         }, 'VirtualDeviceConnectInfo' ),
                 'backing' => bless( {
                                       'useAutoDetect' => '0',
                                       'deviceName' => ''
                                     }, 'VirtualCdromRemoteAtapiBackingInfo' ),
                 'unitNumber' => '0',
                 'deviceInfo' => bless( {
                                          'summary' => 'Remote ATAPI',
                                          'label' => 'CD/DVD drive 1'
                                        }, 'Description' ),
                 'controllerKey' => '201',
                 'key' => '3002'
               }, 'VirtualCdrom' );

