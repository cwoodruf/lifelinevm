#!/usr/bin/perl
# purpose of this script is to play the paidto date for a box
use Lifeline;
use strict;
my $ll = Lifeline->init;
my $a = $ll->{agi};
if ($ll->{paidto}) {
	$a->stream_file('ll-en-paidto');
	$a->exec("Wait","1");
	$a->say_date($ll->{paidto});
} 
	
