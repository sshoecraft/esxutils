#!/usr/bin/perl -w

###############################################################################
# Copyright 2010 Duke University and Apache Software Foundation
#
# Licensed to the Apache Software Foundation (ASF) under one or more
# contributor license agreements.  See the NOTICE file distributed with
# this work for additional information regarding copyright ownership.
# The ASF licenses this file to You under the Apache License, Version 2.0
# (the "License"); you may not use this file except in compliance with
# the License.  You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

###############################################################################

package VCL::Module::Provisioning::esxduke;

# Configure inheritance
use base qw(VCL::Module::Provisioning);

# Specify the version of this module
our $VERSION = '1.00';

# Specify the version of Perl to use
use 5.008000;

use strict;
use warnings;
use diagnostics;

use VCL::utils;
use Fcntl qw(:DEFAULT :flock);

# Use VMware's perl libraries
use VMware::VIRuntime;
use VMware::VILib;
use VMware::VIExt;




##############################################################################

=head1 CLASS ATTRIBUTES

=cut

=head2 %VMWARE_CONFIG

 Data type   : hash
 Description : %VMWARE_CONFIG is a hash containing the general VMWARE
configuration
              for the management node this code is running on. Since the
data is
                                       the same for every instance of the
VMWARE class, a class attribute
                                       is used and the hash is shared among
all instances. This also
                                       means that the data only needs to be
retrieved from the database
                                       once.

=cut

#my %VMWARE_CONFIG;


##############################################################################

=head1 OBJECT METHODS

=cut


#/////////////////////////////////////////////////////////////////////////////

=head2 initialize

 Parameters  :
 Returns     :
 Description :

=cut

# A provisioning object is created for each request.  So this initialize is
done once per request.
sub initialize {
       my $self = shift;
       if (!defined ($self) ) {
               return 0;
       }
       notify($ERRORS{'DEBUG'}, 0, "Duke vmware ESX module initialized");
       return 1;
} ## end sub initialize

sub _checkConnection{
       my $self = shift;
       if (!$VMware::VICommon::is_connected) {
               # Login to the ESX/VC server
               my $retval;
               eval {
                       local $SIG{__DIE__};   # We don't want vcld's die
handler getting this
                       $retval = Util::connect('https://' .
$self-data-get_vmhost_hostname . "/sdk/webService",
$self-data-get_vmhost_profile_username(),
$self-data-get_vmhost_profile_password());
               };
               if ($@ || !$retval) {
                       notify($ERRORS{'CRITICAL'}, 0, "Could not login to "
. $self-data-get_vmhost_hostname . ": " . $@);
                       return 0;
               }
       }
       return 1;
}


#/////////////////////////////////////////////////////////////////////////////

=head2 provision

 Parameters  : hash
 Returns     : 1(success) or 0(failure)
 Description : loads virtual machine with requested image

=cut

sub load {
       my $self = shift;

       #check to make sure this call is for the esx module
       if (ref($self) !~ /esxduke/i) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine was called as a
function, it must be called as a class method");
               return 0;
       }

       my $request_data = shift;
       my ($package, $filename, $line, $sub) = caller(0);
       notify($ERRORS{'DEBUG'}, 0,
"****************************************************");

       # get various useful vars from the database
       my $request_id           = $self-data-get_request_id;
       my $reservation_id       = $self-data-get_reservation_id;
       my $vmhost_hostname      = $self-data-get_vmhost_hostname;
       my $image_name           = $self-data-get_image_name;
       my $computer_shortname   = $self-data-get_computer_short_name;
       my $vmclient_computerid  = $self-data-get_computer_id;
       my $vmclient_imageminram = $self-data-get_image_minram;
       my $image_os_name        = $self-data-get_image_os_name;
       my $image_os_type        = $self-data-get_image_os_type;
       my $image_identity       = $self-data-get_image_identity;
       my $user_name            = $self-data-get_user_login_id . "@" .
$self-data-get_user_affiliation_name;

       my $virtualswitch0   =
$self-data-get_vmhost_profile_virtualswitch0;
       my $virtualswitch1   =
$self-data-get_vmhost_profile_virtualswitch1;
       my $vmclient_eth0MAC = $self-data-get_computer_eth0_mac_address;
       my $vmclient_eth1MAC = $self-data-get_computer_eth1_mac_address;
       my $vmclient_OSname  = $self-data-get_image_os_name;

       my $vmhost_username = $self-data-get_vmhost_profile_username();
       my $vmhost_password = $self-data-get_vmhost_profile_password();

       $vmhost_hostname =~ /([-_a-zA-Z0-9]*)(\.?)/;
       my $vmhost_shortname = $1;

       $self-_checkConnection();

       notify($ERRORS{'OK'},    0, "Entered ESX Duke module, loading
$image_name on $computer_shortname (on $vmhost_hostname) for $user_name
(reservation $reservation_id)");

       my $newVm;
       my $newVmPath;
       my $task;
       my $vmView = $self-_getvmView(0);
       my $imageDatastore;
       my $datacenter;

       my $fileManager = VIExt::get_file_manager();

       # These should be somehow configurable in the future
       my $resourcePoolName = "VCL-TEST";
       my $folderName = "VCL-TEST";
       my $datastoreName = "VCL-TEST-01";

       if(open(CONF, "/etc/vcl/vcld.conf")){
               my  @conf=&lt;CONF;
               close(CONF);
               foreach $line (@conf) {
                 #resourcePool
                 if($line =~ /^RESOURCEPOOLNAME=([\S]*)/){
                               chomp($line);
                               $resourcePoolName=$1;
                       }
                       #folderName
                       if($line =~ /^FOLDERNAME=([\S]*)/){
                               chomp($line);
                               $folderName=$1;
                       }
                       #datastoreName
                       if($line =~ /^DATASTORENAME=([\S]*)/){
                               chomp($line);
                               $datastoreName=$1;
                       }
               }
       }

       # Get some info on our base image
       my $imageView = $self-_getimageView();
       if (! $imageView) {
               return 0;
       }

       my $parentView = Vim::get_view(mo_ref =$imageView-{parent});
       while (ref($parentView) ne 'Datacenter') {
               $parentView = Vim::get_view(mo_ref =
$parentView-{parent});
       }
       $datacenter = $parentView;

       $imageDatastore = Vim::get_view(mo_ref =
@{$imageView-{datastore}}[0]);

       my $resourcePool = Vim::find_entity_view(view_type =
'ResourcePool', filter ={'name' =$resourcePoolName}, begin_entity =
$datacenter);
       if (! defined($resourcePool)) {
               notify($ERRORS{'WARNING'}, 0, "ERROR: esxduke-load could
not find resource pool $resourcePoolName");
               return 0;
       }
       my $folder = Vim::find_entity_view(view_type ='Folder', filter =
{'name' =$folderName}, begin_entity =$datacenter);
       if (! defined($folder)) {
               notify($ERRORS{'WARNING'}, 0, "ERROR: esxduke-load could
not find folder $folderName");
               return 0;
       }

       if ($vmView) {
               notify($ERRORS{'DEBUG'}, 0, "Removing old copy of
$computer_shortname");
               if (!$self-_removeOldVM()) {
                       return 0;
               }
       }

       # Copy file down
       my $service = Vim::get_vim_service();
       my ($mode, $datacenter_foo, $datastore, $filepath) =
VIExt::parse_remote_path($imageView-{config}-{files}-{vmPathName});
       #my ($mode, $datacenter_foo, $datastore, $filepath) =
VIExt::parse_remote_path($imageView-{config}-{files}-{vmPathName});
       $filepath =~ m/(.*)\/.*/;
       my $imageDir = $1;
       my $resp = VIExt::http_get_file($mode, $filepath, $datastore,
$datacenter-{name});
       if (!defined($resp) || !$resp-is_success) {
               notify($ERRORS{'WARNING'}, 0, "ERROR: esxduke-load could
not get config for $image_name");
               return 0;
       }
       # Edit file
       my $newvmx = $resp-content;
       $newvmx =~ s/^displayName = .*$/displayName =
\"$computer_shortname\"/gm;
       $newvmx =~ s/^extendedConfigFile = .*$/extendedConfigFile =
\"$computer_shortname.vmxf\"/gm;
       $newvmx =~ s/^scsi0:0.filename = .*$/scsi0:0.filename = \"FOO\"/gm;
       $newvmx =~ s/^nvram = .*$/nvram = \"$computer_shortname.nvram\"/gm;

       $newvmx =~ s/^uuid.location = .*$//gm;
       $newvmx =~ s/^uuid.bios = .*$//gm;
       $newvmx =~ s/^sched.swap.derivedName = .*$//gm;
       $newvmx =~ s/^sched.mem.max = .*$//gm;
       $newvmx =~ s/^annotation = .*$//gm;
       my $timeStr = localtime;
       $newvmx .= "annotation = \"$image_name created for $user_name at
$timeStr\"\n";

       $newvmx =~ s/^ethernet0.networkName = .*$/ethernet0.networkName =
\"$virtualswitch0\"/gm;
       $newvmx =~ s/^ethernet1.networkName = .*$/ethernet1.networkName =
\"$virtualswitch1\"/gm;

       if (!$VMWARE_MAC_ETH0_GENERATED &amp;&amp; defined($vmclient_eth0MAC)) {
               # Make sure the address type is set right and we have a
.address line
               if ($newvmx !~ /^ethernet0.addressType = \"static\"$/m) {
                       $newvmx .= "ethernet0.addressType = \"static\"\n";
                       $newvmx .= "ethernet0.address = \"XXX\"\n";
               }
               $newvmx =~ s/^ethernet0.address = .*$/ethernet0.address =
\"$vmclient_eth0MAC\"/gm;
       } else {
               $newvmx =~ s/^ethernet0.addressType =
\"static\"$/ethernet0.addressType = \"generated\"/gm;
       }
       if (!$VMWARE_MAC_ETH1_GENERATED &amp;&amp; defined($vmclient_eth1MAC)) {
               # Make sure the address type is set right and we have a
.address line
               if ($newvmx !~ /^ethernet1.addressType = \"static\"$/m) {
                       $newvmx .= "ethernet1.addressType = \"static\"\n";
                       $newvmx .= "ethernet1.address = \"XXX\"\n";
               }
               $newvmx =~ s/^ethernet1.address = .*$/ethernet1.address =
\"$vmclient_eth1MAC\"/gm;
       } else {
               $newvmx =~ s/^ethernet1.addressType =
\"static\"$/ethernet1.addressType = \"generated\"/gm;
       }

       my $imageDSUUID;
       if (defined($imageDatastore-{vmfs}-{uuid})) {
               $imageDSUUID = $imageDatastore-{vmfs}-{uuid};
       } else {
               # Use the name if its NFS, not preferred, but I don't know
how to get the UUID for an NFS datastore
               $imageDSUUID = $imageDatastore-info-name;
       }

       # This is ugly, but I don't know a better way to make sure I catch
all the disks.  If anyone has a better way, please let me know
       my @vmxlines = split(/\n/, $newvmx);
       for $line (@vmxlines) {
               # Make sure the scsi filename isn't an absolute path before
we make it an absolute path
               if ($line !~ /scsi\d:\d{1,2}.fileName = "\/.*"/) {
                       $line =~ s/scsi(\d:\d{1,2}).fileName =
"(.*)"/scsi$1.fileName = "\/vmfs\/volumes\/$imageDSUUID\/$imageDir\/$2"/;
               }
       }
       $newvmx = join("\n", @vmxlines) . "\n";

       # Upload
       # Need to make sure this directory doesn't already exist...
       _deleteDirectory($datastoreName, $computer_shortname, $datacenter);
       eval {
               $fileManager-MakeDirectory(name ="[$datastoreName] " .
$computer_shortname, datacenter =$datacenter);
       };
       if ($@) {
               notify($ERRORS{'WARNING'}, 0, "ERROR: Unable to create
directory [$datastoreName] " . $computer_shortname . ": " .
($@-fault_string));
               return 0;
       }

       # Based on VIExt::http_put_file
       my $serviceURI = URI::URL-new($service-{vim_soap}-{url});
       my $userAgent = $service-{vim_soap}-{user_agent};

       $newVmPath = $computer_shortname . "/" . $computer_shortname .
".vmx";
       my $attempt = 1;
       while ($attempt &lt;= 3) {
               my $req = VIExt::build_http_request("PUT", "folder",
$serviceURI, $newVmPath, $datastoreName, $datacenter-{name});
               $req-header('Content-Type', 'application/octet-stream');
               $req-header('Content-Length', length($newvmx));
               $req-content($newvmx);
               $resp = $userAgent-request($req);
               if (!$resp || !$resp-is_success) {
                       notify($ERRORS{'WARNING'}, 0, "ERROR: Unable to
upload [$datastoreName] $newVmPath: " . $resp-message . ": " .
$resp-content);
                       $attempt += 1;
                       sleep 3;
                       next;
               } else {
                       last;
               }
       }

       if (!$resp || !$resp-is_success) {
               return 0;
       }
       if ($attempt 1) {
               notify($ERRORS{'OK'}, 0, "Uploaded new vmx file on attempt
number $attempt");
       }

       $newVmPath = "[$datastoreName]" . " $newVmPath";

       $attempt = 1;
       my $result;
       while ($attempt &lt;= 3) {
               $task = $folder-RegisterVM_Task(path =$newVmPath, name =
$computer_shortname, asTemplate ="false", pool =$resourcePool);
               $result = _checkTask($task, "registering
$computer_shortname");
               if (!$result) {
                       $attempt += 1;
                       sleep 3;
                       next;
               } else {
                       last;
               }
       }

       if (!$resp) {
               return 0;
       }
       if ($attempt 1) {
               notify($ERRORS{'OK'}, 0, "Registered VM on attempt number
$attempt");
       }

       $vmView = $self-_getvmView();

       $task = $vmView-CreateSnapshot_Task(name ='linked clone from
vcl-image-' . $self-data-get_image_name, memory ="false", quiesce =
"true");
       if (!_checkTask($task, "creating snapshot of $computer_shortname"))
{
               return 0;
       }

       if ($vmclient_OSname =~ /winxp/) {
               # I am hard coding this for now... I am sure we can pass it
in or read it from the db
               _customizeVm('vcl-xp', $computer_shortname , $vmView);
       }

       $task = $vmView-PowerOnVM_Task();
       if (!_checkTask($task, "powering on $computer_shortname")) {
               return 0;
       }


       #Set some variable
       my $wait_loops = 0;
       my $arpstatus  = 0;
       my $client_ip;

       if ($VMWARE_MAC_ETH0_GENERATED) {
               # allowing vmware to generate the MAC address
               # find out what MAC got assigned
               # find out what IP address is assigned to this MAC
               my $devices = $vmView-config-hardware-device;
               my $mac_addr;
               foreach my $dev (@$devices) {
                       next unless ($dev-isa("VirtualEthernetCard"));
                       notify($ERRORS{'DEBUG'}, 0, "deviceinfo-summary:
$dev-deviceinfo-summary");
                       notify($ERRORS{'DEBUG'}, 0, "virtualswitch0:
$virtualswitch0");
                       if ($dev-deviceInfo-summary eq $virtualswitch0) {
                               $mac_addr = $dev-macAddress;
                               last;
                       }
               }
               if (!$mac_addr) {
                       notify($ERRORS{'WARNING'}, 0, "Failed to find MAC
address");
                       return 0;
               }
               notify($ERRORS{'DEBUG'}, 0, "Queried MAC address is
$mac_addr");

               # Query ARP table for $mac_addr to find the IP (waiting for
machine to come up if necessary)
               # The DHCP negotiation should add the appropriate ARP entry
for us
               while (!$arpstatus) {
                       my $arpoutput = `arp -n`;
                       if ($arpoutput =~
/^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}).*?$mac_addr/mi) {
                               $client_ip = $1;
                               $arpstatus = 1;
                               notify($ERRORS{'OK'}, 0,
"$computer_shortname now has ip $client_ip");
                       }
                       else {
                               if ($wait_loops 24) {
                                       notify($ERRORS{'WARNING'}, 0,
"waited acceptable amount of time for dhcp, please check $computer_shortname
on $vmhost_shortname");
                                       return 0;
                               }
                               else {
                                       $wait_loops++;
                                       notify($ERRORS{'OK'}, 0, "going to
sleep 10 seconds, waiting for computer to DHCP. Try $wait_loops");
                                       sleep 10;
                               }
                       } ## end else [ if ($arpoutput =~
/^(\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}).*?$mac_addr/mi)
               } ## end while (!$arpstatus)



               notify($ERRORS{'OK'}, 0, "Found IP address $client_ip");

               # Delete existing entry for $computer_shortname in
/etc/hosts (if any)
               notify($ERRORS{'OK'}, 0, "Removing old hosts entry");
               my $sedoutput = `sed -i "/.*\\b$computer_shortname\$/d"
/etc/hosts`;
               notify($ERRORS{'DEBUG'}, 0, $sedoutput);

               # Add new entry to /etc/hosts for $computer_shortname
               `echo -e "$client_ip\t$computer_shortname" /etc/hosts`;
       } ## end if ($VMWARE_MAC_ETH0_GENERATED)
       else {
               notify($ERRORS{'DEBUG'}, 0, "IP is known for
$computer_shortname");
       }
       # Start waiting for SSH to come up
       my $sshdstatus = 0;
       $wait_loops = 0;
       my $sshd_status = "off";
       notify($ERRORS{'DEBUG'}, 0, "Waiting for ssh to come up on
$computer_shortname");
       while (!$sshdstatus) {
               $sshd_status = _sshd_status($computer_shortname,
$image_name, $image_os_type);
               if ($sshd_status eq "on") {
                       $sshdstatus = 1;
                       notify($ERRORS{'OK'}, 0, "$computer_shortname now
has active sshd running");
               }
               else {
                       #either sshd is off or N/A, we wait
                       if ($wait_loops 150) {
                               notify($ERRORS{'WARNING'}, 0, "waited
acceptable amount of time for sshd to become active, please check
$computer_shortname on $vmhost_shortname");
                               #need to check power, maybe reboot it. for
now fail it
                               return 0;
                       }
                       else {
                               $wait_loops++;
                               # to give post config a chance
                               notify($ERRORS{'DEBUG'}, 0, "going to sleep
10 seconds, waiting for computer to start SSH. Try $wait_loops");
                               sleep 10;
                       }
               }    # else
       }    #while

       # Set IP info
       if ($IPCONFIGURATION ne "manualDHCP") {
               #not default setting
               if ($IPCONFIGURATION eq "dynamicDHCP") {
                       insertloadlog($reservation_id, $vmclient_computerid,
"dynamicDHCPaddress", "collecting dynamic IP address for node");
                       notify($ERRORS{'DEBUG'}, 0, "Attempting to query
vmclient for its public IP...");
#### Sean
### Why call getdynamicaddress?  It doesn't work.  Should we instead just
write a local method
#### to get the ip from another means?
#### Liz
                       my $assignedIPaddress =
getdynamicaddress($computer_shortname, $vmclient_OSname, $image_os_type);
                       if ($assignedIPaddress) {
                               #update computer table
                               notify($ERRORS{'DEBUG'}, 0, " Got dynamic
address from vmclient, attempting to update database");
                               if
(update_computer_address($vmclient_computerid, $assignedIPaddress)) {
                                       notify($ERRORS{'DEBUG'}, 0, "
succesfully updated IPaddress of node $computer_shortname");
                               }
                               else {
                                       notify($ERRORS{'CRITICAL'}, 0,
"could not update dynamic address $assignedIPaddress for $computer_shortname
$image_name");
                                       return 0;
                               }
                       } ## end if ($assignedIPaddress)
                       else {
                               notify($ERRORS{'CRITICAL'}, 0, "could not
fetch dynamic address from $computer_shortname $image_name");
                               insertloadlog($reservation_id,
$vmclient_computerid, "failed", "could not collect dynamic IP address for
node");
                               return 0;
                       }
               } ## end if ($IPCONFIGURATION eq "dynamicDHCP")
               elsif ($IPCONFIGURATION eq "static") {
                       notify($ERRORS{'CRITICAL'}, 0, "STATIC ASSIGNMENT
NOT SUPPORTED. See vcld.conf");
                       return 0;
                       #insertloadlog($reservation_id,
$vmclient_computerid, "staticIPaddress", "setting static IP address for
node");
                       #if (setstaticaddress($computer_shortname,
$vmclient_OSname, $vmclient_publicIPaddress)) {
                       #       # good set static address
                       #}
               }
       } ## end if ($IPCONFIGURATION ne "manualDHCP")

       # Perform post load tasks
       return 1;
} ## end sub load

#/////////////////////////////////////////////////////////////////////////

=head2 node_status

 Parameters  : $nodename, $log
 Returns     : array of related status checks
 Description : checks on sshd, currentimage

=cut

sub node_status {
       my $self = shift;

       my ($package, $filename, $line, $sub) = caller(0);

       my $vmpath             = 0;
       my $datastorepath      = 0;
       my $requestedimagename = 0;
       my $vmhost_type        = 0;
       my $vmhost_hostname    = 0;
       my $vmhost_imagename   = 0;
       my $image_os_type      = 0;
       my $vmclient_shortname = 0;
       my $request_forimaging = 0;
       my $identity_keys      = 0;
       my $log                = 0;
       my $computer_node_name = 0;


       # Check if subroutine was called as a class method
       if (ref($self) !~ /esxduke/i) {
               notify($ERRORS{'OK'}, 0, "subroutine was called as a
function");
               if (ref($self) eq 'HASH') {
                       $log = $self-{logfile};
                       #notify($ERRORS{'DEBUG'}, $log, "self is a hash
reference");

                       $vmpath             =
$self-{vmhost}-{vmprofile}-{vmpath};
                       $datastorepath      =
$self-{vmhost}-{vmprofile}-{datastorepath};
                       $requestedimagename =
$self-{imagerevision}-{imagename};
                       $vmhost_type        =
$self-{vmhost}-{vmprofile}-{vmtype}-{name};
                       $vmhost_hostname    = $self-{vmhost}-{hostname};
                       $vmhost_imagename   = $self-{vmhost}-{imagename};
                       $image_os_type      = $self-{image}-{OS}-{type};
                       $computer_node_name = $self-{computer}-{hostname};
                       $identity_keys      =
$self-{managementnode}-{keys};

               } ## end if (ref($self) eq 'HASH')
                   # Check if node_status returned an array ref
               elsif (ref($self) eq 'ARRAY') {
                       notify($ERRORS{'DEBUG'}, $log, "self is a array
reference");
               }

               $vmclient_shortname = $1 if ($computer_node_name =~
/([-_a-zA-Z0-9]*)(\.?)/);
       } ## end if (ref($self) !~ /esx/i)
       else {

               # try to contact vm
               # $self-data-get_request_data;
               # get state of vm
               $vmpath             =
$self-data-get_vmhost_profile_vmpath;
               $datastorepath      =
$self-data-get_vmhost_profile_datastore_path;
               $requestedimagename = $self-data-get_image_name;
               $vmhost_type        = $self-data-get_vmhost_type;
               $vmhost_hostname    = $self-data-get_vmhost_hostname;
               $vmhost_imagename   = $self-data-get_vmhost_image_name;
               $image_os_type      = $self-data-get_image_os_type;
               $vmclient_shortname = $self-data-get_computer_short_name;
               $request_forimaging = $self-data-get_request_forimaging();
       } ## end else [ if (ref($self) !~ /esx/i)

       notify($ERRORS{'OK'},    0, "Entering node_status, checking status
of $vmclient_shortname");
       notify($ERRORS{'DEBUG'}, 0, "request_for_imaging:
$request_forimaging");
       notify($ERRORS{'DEBUG'}, 0, "requeseted image name:
$requestedimagename");

       my ($hostnode, $identity);

       # Create a hash to store status components
       my %status;

       # Initialize all hash keys here to make sure they're defined
       $status{status}       = 0;
       $status{currentimage} = 0;
       $status{ping}         = 0;
       $status{ssh}          = 0;
       $status{vmstate}      = 0;    #on or off
       $status{image_match}  = 0;

       if ($vmhost_type eq "blade") {
               $hostnode = $1 if ($vmhost_hostname =~
/([-_a-zA-Z0-9]*)(\.?)/);
               $identity = $IDENTITY_bladerhel;
 #if($vm{vmhost}{imagename} =~ /^(rhel|rh3image|rh4image|fc|rhfc)/);
       }
       else {
               #using FQHN
               $hostnode = $vmhost_hostname;
               $identity = $IDENTITY_linux_lab if ($vmhost_imagename =~
/^(realmrhel)/);
       }

       if (!$identity) {
               notify($ERRORS{'CRITICAL'}, 0, "could not set ssh identity
variable for image $vmhost_imagename type= $vmhost_type host=
$vmhost_hostname");
       }

       # Check if node is pingable
       notify($ERRORS{'DEBUG'}, 0, "checking if $vmclient_shortname is
pingable");
       if (_pingnode($vmclient_shortname)) {
               $status{ping} = 1;
               notify($ERRORS{'OK'}, 0, "$vmclient_shortname is pingable
($status{ping})");
       }
       else {
               notify($ERRORS{'OK'}, 0, "$vmclient_shortname is not
pingable ($status{ping})");
               $status{status} = 'RELOAD';
               return $status{status};
       }

       #
       #my $vmx_directory = "$requestedimagename$vmclient_shortname";
       #my $myvmx         =
"$vmpath/$requestedimagename$vmclient_shortname/$requestedimagename$vmclient_shortname.vmx";
       #my $mybasedirname = $requestedimagename;
       #my $myimagename   = $requestedimagename;

       notify($ERRORS{'DEBUG'}, 0, "Trying to ssh...");

       #can I ssh into it
       my $sshd = _sshd_status($vmclient_shortname, $requestedimagename,
$image_os_type);


       #is it running the requested image
       if ($sshd eq "on") {

               notify($ERRORS{'DEBUG'}, 0, "SSH good, trying to query image
name");

               $status{ssh} = 1;
               $identity = $IDENTITY_bladerhel;
               my @sshcmd = run_ssh_command($vmclient_shortname, $identity,
"cat currentimage.txt");
               $status{currentimage} = $sshcmd[1][0];

               notify($ERRORS{'DEBUG'}, 0, "Image name:
$status{currentimage}");

               if ($status{currentimage}) {
                       chomp($status{currentimage});
                       if ($status{currentimage} =~ /$requestedimagename/)
{
                               $status{image_match} = 1;
                               notify($ERRORS{'OK'}, 0,
"$vmclient_shortname is loaded with requestedimagename
$requestedimagename");
                       }
                       else {
                               notify($ERRORS{'OK'}, 0,
"$vmclient_shortname reports current image is currentimage=
$status{currentimage} requestedimagename= $requestedimagename");
                       }
               } ## end if ($status{currentimage})
       } ## end if ($sshd eq "on")

       # Determine the overall machine status based on the individual
status results
       if ($status{ssh} &amp;&amp; $status{image_match}) {
               $status{status} = 'READY';
       }
       else {
               $status{status} = 'RELOAD';
       }

       notify($ERRORS{'DEBUG'}, 0, "status set to $status{status}");


       if ($request_forimaging) {
               $status{status} = 'RELOAD';
               notify($ERRORS{'OK'}, 0, "request_forimaging set, setting
status to RELOAD");
       }

       notify($ERRORS{'DEBUG'}, 0, "returning node status hash reference
(\$node_status-{status}=$status{status})");
       return \%status;

} ## end sub node_status

sub does_image_exist {
       my $self = shift;
       if (ref($self) !~ /esxduke/i) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine was called as a
function, it must be called as a class method");
               return 0;
       }

       my $image_view = $self-_getimageView(0);
       if (! $image_view) {
               return 0;
       }
       return 1;

} ## end sub does_image_exist


#/////////////////////////////////////////////////////////////////////////////

=head2  getimagesize

 Parameters  : imagename
 Returns     : 0 failure or size of image
 Description : in size of Kilobytes

=cut

# Need to implement
sub get_image_size {
       my $self = shift;
       if (ref($self) !~ /esxduke/i) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine was called as a
function, it must be called as a class method");
               return 0;
       }

       # Just a placeholder - not sure if this value even matters
       return 1;
} ## end sub get_image_size

sub power_off {
       my $self = shift;
       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }

       my $task;
       my $computer_shortname = $self-data-get_computer_short_name;
       my $vmView = $self-_getvmView();
       if (!$vmView) {
               return 0;
       }
       if ($vmView-{runtime}-{powerState}-val eq "poweredOff") {
               # VM is turned off, so just return happily
               return 1;
       }
       $task = $vmView-PowerOffVM_Task();
       if (!_checkTask($task, "powering off " . $computer_shortname)) {
               return 0;
       }
       return 1;
}

sub power_on {
       my $self = shift;
       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }

       my $task;
       my $computer_shortname = $self-data-get_computer_short_name;
       my $vmView = $self-_getvmView();
       if (!$vmView) {
               return 0;
       }
       if ($vmView-{runtime}-{powerState}-val eq "poweredOn") {
               # VM is turned on, so just return happily
               return 1;
       }
       $task = $vmView-PowerOnVM_Task();
       if (!_checkTask($task, "powering on " . $computer_shortname)) {
               return 0;
       }
       return 1;
}

sub power_reset {
       my $self = shift;
       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }

       my $task;
       my $computer_shortname = $self-data-get_computer_short_name;
       my $vmView = $self-_getvmView();
       if (!$vmView) {
               return 0;
       }
       $task = $vmView-ResetVM_Task();
       if (!_checkTask($task, "reseting on " . $computer_shortname)) {
               return 0;
       }
       return 1;
}

sub power_status {
       my $self = shift;
       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }

       my $task;
       my $computer_shortname = $self-data-get_computer_short_name;
       my $vmView = $self-_getvmView();
       if (!$vmView) {
               return;
       }
       $vmView-update_view_data();
       if ($vmView-runtime-powerState-val eq 'poweredOn') {
               return "on";
       } elsif ($vmView-runtime-powerState-val eq 'poweredOff') {
               return "off";
       } else {
               notify($ERRORS{'WARNING'}, 0, "Unable to determin power
status.  VMware reports powerState: " . $vmView-runtime-powerState-val);
               return;
       }
}


#/////////////////////////////////////////////////////////////////////////////

=head2 capture

 Parameters  : $request_data_hash_reference
 Returns     : 1 if sucessful, 0 if failed
 Description : Creates a new vmware image.

=cut

sub capture {
       my $self = shift;
       if (ref($self) !~ /esxduke/i) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine was called as a
function, it must be called as a class method");
               return 0;
       }

       my ($package, $filename, $line, $sub) = caller(0);

       # Store some hash variables into local variables
       # to pass to write_current_image routine
       my $request_data = $self-data-get_request_data;

       if (!$request_data) {
               notify($ERRORS{'WARNING'}, 0, "unable to retrieve request
data hash");
               return 0;
       }

       # Store some hash variables into local variables
       my $request_id     = $self-data-get_request_id;
       my $reservation_id = $self-data-get_reservation_id;

       my $image_id       = $self-data-get_image_id;
       my $image_os_name  = $self-data-get_image_os_name;
       my $image_identity = $self-data-get_image_identity;
       my $image_os_type  = $self-data-get_image_os_type;
       my $image_name     = $self-data-get_image_name();

       my $computer_id        = $self-data-get_computer_id;
       my $computer_shortname = $self-data-get_computer_short_name;
       my $computer_nodename  = $computer_shortname;
       my $computer_hostname  = $self-data-get_computer_hostname;
       my $computer_type      = $self-data-get_computer_type;

       my $vmtype_name             = $self-data-get_vmhost_type_name;
       my $vmhost_vmpath           =
$self-data-get_vmhost_profile_vmpath;
       my $vmprofile_vmdisk        =
$self-data-get_vmhost_profile_vmdisk;
       my $vmprofile_datastorepath =
$self-data-get_vmhost_profile_datastore_path;
       my $vmhost_hostname         = $self-data-get_vmhost_hostname;
       my $host_type               = $self-data-get_vmhost_type;
       my $vmhost_imagename        = $self-data-get_vmhost_image_name;

       my ($hostIdentity, $hostnodename);
       if ($host_type eq "blade") {
               $hostnodename = $1 if ($vmhost_hostname =~
/([-_a-zA-Z0-9]*)(\.?)/);
               $hostIdentity = $IDENTITY_bladerhel;
       }
       else {
               #using FQHN
               $hostnodename = $vmhost_hostname;
               $hostIdentity = $IDENTITY_linux_lab if ($vmhost_imagename =~
/^(realmrhel)/);
       }
       # Assemble a consistent prefix for notify messages
       my $notify_prefix = "req=$request_id, res=$reservation_id:";


       # Print some preliminary information
       notify($ERRORS{'OK'}, 0, "$notify_prefix new name: $image_name");
       notify($ERRORS{'OK'}, 0, "$notify_prefix computer_name:
$computer_shortname");
       notify($ERRORS{'OK'}, 0, "$notify_prefix vmhost_hostname:
$vmhost_hostname");
       notify($ERRORS{'OK'}, 0, "$notify_prefix vmtype_name:
$vmtype_name");

       my $vmView = $self-_getvmView();
       if (!$vmView) {
               return 0;
       }

       my $imageFolderName = "VCL-TEST";
       my $imageDatastoreName = "VCL-IMAGES-TEST-01";

       if(open(CONF, "/etc/vcl/vcld.conf")){
               my  @conf=&lt;CONF;
               close(CONF);
               foreach $line (@conf) {
                       #folderName
                       if($line =~ /^IMAGEFOLDERNAME=([\S]*)/){
                               chomp($line);
                               $imageFolderName=$1;
                       }
                       #datastoreName
                       if($line =~ /^IMAGEDATASTORENAME=([\S]*)/){
                               chomp($line);
                               $imageDatastoreName=$1;
                       }
               }
       }

       my $parentView = Vim::get_view(mo_ref =$vmView-{parent});
       while (ref($parentView) ne 'Datacenter') {
               $parentView = Vim::get_view(mo_ref =
$parentView-{parent});
       }
       my $datacenter = $parentView;
       my $datastoreView;
       my $datastores = Vim::get_views(mo_ref_array =
$datacenter-{datastore});
       foreach (@$datastores) {
               if ($_-{info}-{name} eq $imageDatastoreName) {
                       $datastoreView = $_;
                       last;
               }
       }
       if (!defined($datastoreView)) {
               notify($ERRORS{'CRITICAL'}, 0, "Unable to find datastore
$imageDatastoreName");
               return 0;
       }
       my $folder = Vim::find_entity_view(view_type ='Folder', filter =
{'name' =$imageFolderName}, begin_entity =$datacenter);
       if (! defined($folder)) {
               notify($ERRORS{'CRITICAL'}, 0, "ERROR: esxduke-capture
could not find folder $imageFolderName");
               return 0;
       }

       # Modify currentimage.txt
       if (write_currentimage_txt($self-data)) {
               notify($ERRORS{'OK'}, 0, "$notify_prefix currentimage.txt
updated on $computer_shortname");
       }
       else {
               notify($ERRORS{'WARNING'}, 0, "$notify_prefix unable to
update currentimage.txt on $computer_shortname");
               return 0;
       }

       # Set some vm paths and names
       my $vmx_directory  = "$reservation_id$computer_shortname";
       my $vmx_image_name = "$reservation_id$computer_shortname";
       my $vmx_path       =
"$vmhost_vmpath/$vmx_directory/$vmx_image_name.vmx";

       my @sshcmd;

       # Check if pre_capture() subroutine has been implemented by the OS
module
       if ($self-os-can("pre_capture")) {
               # Call OS pre_capture() - it should perform all OS steps
necessary to capture an image
               # pre_capture() should shut down the computer when it is
done
               notify($ERRORS{'OK'}, 0, "calling OS module's pre_capture()
subroutine");

               if (!$self-os-pre_capture({end_state ='off'})) {
                       notify($ERRORS{'WARNING'}, 0, "OS module
pre_capture() failed");
                       return 0;
               }

       }
       # Get the power status, make sure computer is off
       my $power_status = $self-power_status();
       notify($ERRORS{'DEBUG'}, 0, "retrieved power status:
$power_status");
       if ($power_status eq 'off') {
               notify($ERRORS{'OK'}, 0, "verified $computer_nodename power
is off");
       }
       elsif ($power_status eq 'on') {
               notify($ERRORS{'WARNING'}, 0, "$computer_nodename power is
still on, turning computer off");

               # Attempt to power off computer
               if ($self-power_off()) {
                       notify($ERRORS{'OK'}, 0, "$computer_nodename was
powered off");
               }
               else {
                       notify($ERRORS{'WARNING'}, 0, "failed to power off
$computer_nodename");
                       return 0;
               }
       }
       else {
               notify($ERRORS{'WARNING'}, 0, "failed to determine power
status of $computer_nodename");
               return 0;
       }

       # Clone with new image name: $image_name

       my $relocateSpec = VirtualMachineRelocateSpec-new(datastore =
$datastoreView, transform =
VirtualMachineRelocateTransformation-new('sparse'));
       my $cloneSpec = VirtualMachineCloneSpec-new(powerOn =0, template
=1, location =$relocateSpec);

       notify($ERRORS{'OK'}, 0, "Cloning $computer_shortname to
vcl-image-$image_name");

       my $task = $vmView-CloneVM_Task(folder =$folder, name =
"vcl-image-" . $image_name, spec =$cloneSpec);
       return _checkTask($task, "Cloning $computer_shortname");

} ## end sub capture


sub _getvmView {
       my $self = shift;
       my $is_warning = shift;
       my $notify_level = $ERRORS{'WARNING'};

       if (defined($is_warning) &amp;&amp; !$is_warning) {
               $notify_level = $ERRORS{'DEBUG'};
       }

       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }
       if (defined($self-{_vmView})) {
               return $self-{_vmView};
       }

       $self-_checkConnection();
       my $computer_shortname   = $self-data-get_computer_short_name;
       my $vmView = Vim::find_entity_view(view_type ='VirtualMachine',
filter ={'name' =$computer_shortname });
       if (!$vmView) {
               notify($notify_level, 0, "Could not find VM
$computer_shortname");
               return 0;
       }
       $self-{_vmView} = $vmView;

       return $vmView;
}

sub _getimageView {
       my $self = shift;
       my $is_crit = shift;
       my $notify_level = $ERRORS{'CRITICAL'};

       if (defined($is_crit) &amp;&amp; !$is_crit) {
               $notify_level = $ERRORS{'DEBUG'};
       }

       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }
       if (defined($self-{_imageView})) {
               return $self-{_imageView};
       }

       $self-_checkConnection();
       my $image_name = $self-data-get_image_name;
       my $imageView = Vim::find_entity_view(view_type ='VirtualMachine',
filter ={'name' ="vcl-image-$image_name"});
       if (!$imageView) {
               notify($notify_level, 0, "Could not find image
vcl-image-$image_name");
               return 0;
       }
       if ($imageView-{snapshot}) {
               # This is always crit. If we're called from does_image_exist
we want to log the weird state of the image existing but being invalid
               notify($ERRORS{'CRITICAL'}, 0, "ERROR: vcl-image-$image_name
is an invalid image, it has a snapshot");
               return 0;
       }
       $self-{_imageView} = $imageView;

       return $imageView;
}

sub _removeOldVM {

       my $self = shift;
       unless (ref($self) &amp;&amp; $self-isa('VCL::Module')) {
               notify($ERRORS{'CRITICAL'}, 0, "subroutine can only be
called as a VCL::Module module object method");
               return 0;
       }

       my $task;
       my (@deviceSpecs, $vmDevices, $vmConfigSpec);

       my $vmView = $self-_getvmView();

       if ($vmView-{runtime}-{powerState}-val ne "poweredOff") {
               notify($ERRORS{'DEBUG'}, 0, "Powering off " .
$vmView-{name});
               $task = $vmView-PowerOffVM_Task();
               if (!_checkTask($task, "powering off " . $vmView-{name})) {
                       return 0;
               }
       }

       # We simply unregister the VM.  The _deleteDirectory function will
       # clean up the files.  If we were to Destroy the VM, VMware would
       # wipe out the disk of the image, which we need to avoid.
       $vmView-UnregisterVM();

       $self-{_vmView} = undef;
       return 1;

}

sub _checkTask {
       my ($task, $taskName) = @_;

       my $taskView = Vim::get_view(mo_ref =$task);
       while (1) {
               # Sometimes FileManager-DeleteDatastoreFile() gives us an
odd task, so this is to give us a shot of making it a normal task
               if (!defined $taskView-{info}) {
                       my $attempt = 1;
                       while ($attempt &lt;= 3) {
                               $taskView-update_view_data();
                               if (defined $taskView-{info}) {
                                       last;
                               } else {
                                       $attempt += 1;
                                       sleep 1;
                               }
                       }
                       if (!defined $taskView-{info}) {
                               notify($ERRORS{'WARNING'}, 0, "ERROR: Could
not get valid task for $taskName");
                               return 0;
                       }
               }

               if ($taskView-info-state-val eq "success") {
                       return 1;
               } elsif ($taskView-info-state-val eq "error") {
                       notify($ERRORS{'WARNING'}, 0, "ERROR: $taskName: " .
$taskView-info-error-localizedMessage);
                       return 0;
               }
               sleep 1;
               $taskView-update_view_data();
       }
}

sub _customizeVm {
       my ($customSpec, $nodeName, $view ) = @_;

       my $customView = Vim::get_view(mo_ref =
Vim::get_service_content()-customizationSpecManager);
       my $customizationSpec = $customView-GetCustomizationSpec ('name' =
$customSpec)-spec;

       # We need to set the machine name in this spec
       _setMachineName($nodeName, $customizationSpec);
       my $task = $view-CustomizeVM_Task(spec =$customizationSpec);
       if (_checkTask($task, "customizing $nodeName")){
               notify($ERRORS{'OK'}, 0, "success setting custom specs");
       }
}

sub _setMachineName {
       my ($vmName, $customizationSpec) = @_;

       my @machineName = split(/\./,$vmName);
       my $cust_name = CustomizationFixedName-new (name =
$machineName[0]);
       $customizationSpec-identity-userData-computerName ($cust_name);
       return;
}

sub _deleteDirectory {
       my ($dsName, $dirname, $datacenter) = @_;
       my $ds;

       foreach my $ds2 (@{$datacenter-{'datastore'}}) {
               my $ds_view = Vim::get_view(mo_ref =$ds2);
               if ($ds_view-info-name eq $dsName) {
                       $ds = $ds_view;
                       last;
               }
       }

       if (!defined($ds)) {
               notify($ERRORS{'WARNING'}, 0, "Could not find datastore
$dsName");
               return 0;
       }

       my $ds_browser = Vim::get_view(mo_ref =$ds-browser);

       my $search_spec = HostDatastoreBrowserSearchSpec-new(matchPattern
=[$dirname]);
       my $browse_result = $ds_browser-SearchDatastore(datastorePath =
"[$dsName]", searchSpec =$search_spec);
       # Only do the delete if the file exists
       if (defined $browse_result-file) {
               eval {
                       my $fileManager = Vim::get_view(mo_ref =
Vim::get_service_content()-fileManager);
                       my $task =
$fileManager-DeleteDatastoreFile_Task(name ="[$dsName] $dirname",
datacenter =$datacenter);
                       if (!_checkTask($task, "deleting [$dsName]
$dirname")) {
                               return 0;
                       }
               };
               if ($@) {
                       notify($ERRORS{'WARNING'}, 0, "Error trying to
delete [$dsName] $dirname: " . ($@-fault_string));
               }
       }
}



initialize();

END {
       Util::disconnect();
}


#/////////////////////////////////////////////////////////////////////////////

1;
__END__

=head1 SEE ALSO

L&lt;http://cwiki.apache.org/VCL/

