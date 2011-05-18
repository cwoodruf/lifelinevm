#!/usr/bin/perl
# purpose of this script is to save a security code
use Lifeline;
use strict;
my $ll = Lifeline->init(); 
my ($seccode1,$seccode2,$callerid) = @ARGV;
if ($ll->save_seccode($seccode1,$seccode2)) {
	$ll->log_calls('ll-saveseccode.pl','ok',$callerid);
	$ll->{agi}->set_priority(100);
}
print STDERR "err: $ll->{err}\n";
