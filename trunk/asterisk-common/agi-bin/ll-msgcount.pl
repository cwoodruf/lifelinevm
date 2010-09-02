#!/usr/bin/perl
# purpose of this script is to set the number of messages - don't want to do this every time ll is initiated
use Lifeline;
use strict;
my $ll = Lifeline->init(); 
$ll->load_msgs;

$ll->{msgcount} = 0;
if (defined $ll->{msgs}->{list}) {
	$ll->{msgcount} = scalar @{$ll->{msgs}->{list}};
} 
$ll->set("msgcount",$ll->{msgcount});

