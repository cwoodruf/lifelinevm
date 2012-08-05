#!/usr/bin/perl
# purpose of this script is to check the paidto date / status before checking for the greeting
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need cached messages list
my $paidto_cutoff = shift;
my $callerid = shift;
if ($ll->valid) {
	my $mustmatch = $ll->get('mustmatch');
	if (defined $mustmatch and $ll->{llphone} ne $mustmatch) {
		$ll->log_calls('ll-valid.pl','disabled',$callerid);

	} elsif ($ll->check_paidto($paidto_cutoff)) {
		$ll->log_calls('ll-valid.pl','valid',$callerid);
		$ll->{agi}->set_priority(100);
	} else {
		# never usually gets here
		$ll->log_calls('ll-valid.pl','expired',$callerid);
	}
} else {
	$ll->log_calls('ll-valid.pl','invalid',$callerid);
}

