#!/usr/bin/perl -w
use strict;
use warnings;
use VMware::VIRuntime;
use Data::Dumper;

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
Util::connect();

  my $vmViews  = Vim::find_entity_views( view_type => 'VirtualMachine', filter => { 'name' => $vmname });

  die "vmViews isn't viewable!\n" if ! defined $vmViews;

  foreach my $thisVM ( @$vmViews )
  {
#	print Dumper($thisVM->summary->config);
#	printf("name: %s\n", $thisVM->summary->config->name);
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
