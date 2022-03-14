#!/usr/bin/perl -w

##########################################################################
## OpenAPRS.Net                                                         ##
##########################################################################
## OpenAPRS, Perl Direct Client Connection - Client                     ##
## Copyright (C) 2008 Gregory A. Carter                                 ##
##                                                                      ##
## This program is free software; you can redistribute it and/or modify ##
## it under the terms of the GNU General Public License as published by ##
## the Free Software Foundation; either version 1, or (at your option)  ##
## any later version.                                                   ##
##                                                                      ##
## This program is distributed in the hope that it will be useful,      ##
## but WITHOUT ANY WARRANTY; without even the implied warranty of       ##
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        ##
## GNU General Public License for more details.                         ##
##                                                                      ##
## You should have received a copy of the GNU General Public License    ##
## along with this program; if not, write to the Free Software          ##
## Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            ##
##########################################################################

use Socket;
use Switch;
use Getopt::Long;
use Time::HiRes qw(usleep);

###
# Edit the following two variables if you have an actual
# account with OpenAPRS.
#
# To signup for an account point your web browser at
# http://www.openaprs.net/ and click the "Options" icon
# at the top, then "Signup" in the popup window.
#
# Signing up and verifying an account with OpenAPRS will
# allow you to send positions, objects and APRS messages
# from both the OpenAPRS web page and DCC interface.
#

my $email = "guest";
my $pass = "guest";
my $client = "OpenAPRS-Perl-1.0";

my $result = GetOptions ("email=s" => \$email, # numeric
                         "pass=s" => \$pass, # string
                         "client=s" => \$client, # string
                         "verbose" => \$verbose); # flag

###
# To filter the live output down to a specific area, change
# the following lines.
#

$filter = 0;
$lat = 38.00;
$lon = -122.00;
$range = 50; # in kilometers

###
# Display live data in NMEA waypoints.
#
# This example client can easly be turned into a full
# client that is able to parse and send data to the server
# with a few modificaitons.  To parse a DCC live data stream
# in realtime, set the following to 0 and see `case "318"'
# below in the code.
#

$nmea = 0;

###
# Shouldn't need to edit anything past this line unless you want
# to parse things differently.
#

#############################################################################

$remote_host = "localhost";
$remote_port = 2621;

print "*** Connecting to " . $remote_host . ":" . $remote_port . "\n";

# create a socket
socket(Server, PF_INET, SOCK_STREAM, getprotobyname('tcp'));

# build the address of the remote machine
$internet_addr = inet_aton($remote_host)
    or die "Couldn't convert $remote_host into an Internet address: $!\n";
$paddr = sockaddr_in($remote_port, $internet_addr);

# connect
if (!connect(Server, $paddr)) {
  die "Couldn't connect to $remote_host:$remote_port: $!\n";
} # if

select((select(Server), $| = 1)[0]);  # enable command buffering

# read the remote answer
$i=0;
while(<Server>) {
  ($r, $m) = split(/ /, $_, 2);
  print $_;

  last if ($i > 100);

  switch($r) {
    ###
    # Received welcome, send our login information.
    #
    case "002" {
      Send(".LN $email $pass $client\n");
    } # case 002
    ###
    # Server pinged us, we must respond with .PN within 180 seconds
    # or connection will be timed out.
    #
    case "106" {
      Send(".PN\n");
    } # case 106
    ###
    # We've logged in successfully, now let's tell the server
    # to send us live data.
    #
    case "100" {
#      Send(".QU\n");
      Send(".LC CN:$lat,$lon|RG:25\n");
#      Send(".LV\n");
    } # case 100
    case "305" {
#      Send(".LC CN:$lat,$lon|RG:25\n");
      Send(".QU\n");
    }
  } # switch

  $i++;
} # while

print "*** Disconnected from " . $remote_host . "\n";

# terminate the connection when done
close(Server);

sub Send {
  my($message) = @_;

  print Server "$message";
  print "> $message";
} # sub Send

my $testNum = 0;

sub RunTest {
  Send(".LP CL:NV6G\n");
}

sub CheckResult {
}

sub Vars {
  my($parseMe) = @_;
  my $field;
  my $name;

  $parseMe =~ s/\r//g;
  $parseMe =~ s/\n//g;

  %varMap = ();

  while(length($parseMe)) {
    $name = "";

    for($pos=0; $pos < length($parseMe) && substr($parseMe, $pos, 1) ne ':'; $pos++) {
      $name .= substr($parseMe, $pos, 1);
    } # for

    $parseMe = substr($parseMe, $pos+1);

    #print "name($name)\n";
    #print "parseMe($parseMe)\n";

    $field = "";
    $isEscaped = 0;
    for($pos=0; $pos < length($parseMe); $pos++) {
      $p = substr($parseMe, $pos, 1);

      if (!$isEscaped && ord($p) == 124) {
        last;
      } # if

      if (!$isEscaped && ord($p) == 92) {
        $isEscaped = 1;
      } # if
      else {
        $isEscaped = 0;
        $field .= $p;
      } # else
    } # for

    if (length($parseMe) >= $pos+1) {
      $parseMe = substr($parseMe, $pos+1);
    } # if
    else {
      $parseMe = "";
    } # else

    #print "field($field)\n";

    if (length($name) > 0 && length($field) > 0) {
      $varMap{$name} = $field;
    } # if

  } # while

  return %varMap;
} # sub Vars
