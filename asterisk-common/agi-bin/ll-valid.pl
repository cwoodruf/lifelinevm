#!/usr/bin/perl
# purpose of this script is to load the greeting variable with the correct greeting file
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need cached messages list
my $paidto_cutoff = shift;
if ($ll->valid and $ll->check_paidto($paidto_cutoff)) {
	$ll->{agi}->set_priority(100);
} 
