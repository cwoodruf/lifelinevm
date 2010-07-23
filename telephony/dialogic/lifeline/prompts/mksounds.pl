#!/usr/bin/perl
# purpose of this script is to convert existing dialogic prompts to gsm similar to asterisk-common/bin/convertvox.pl
use strict;
my $inrate = 6000; # dialogic vox sample rate
my $outrate = 8000; # gsm sample rate

while (<DATA>) {
	chomp;
	(my $infile = $_) =~ s/ll-en-(\w+)\.gsm/\1.vox/;
	unless (-f $infile) {
		print "$infile does not exist!\n";
		next;
	}
	my $cmd = "sox -v .7 -t .vox -r $inrate $infile -r $outrate $_ gain -9";
	print "$cmd\n";
	system $cmd and die "error: $!";
}

__DATA__
ll-en-delete.gsm
ll-en-delmsg.gsm
ll-en-extplay.gsm
ll-en-geninstr-features.gsm
ll-en-geninstr.gsm
ll-en-geninstr-intro.gsm
ll-en-getbox.gsm
ll-en-greeting.gsm
ll-en-grtintro.gsm
ll-en-hangup.gsm
ll-en-invalid.gsm
ll-en-lastmsg.gsm
ll-en-listen.gsm
ll-en-message.gsm
ll-en-messages.gsm
ll-en-msgrec.gsm
ll-en-newmsgs.gsm
ll-en-newpin.gsm
ll-en-nomsgs.gsm
ll-en-paidto.gsm
ll-en-prompt.gsm
ll-en-recgrt.gsm
ll-en-reenter.gsm
ll-en-remind.gsm
ll-en-restmsg.gsm
ll-en-restore.gsm
ll-en-thanks.gsm
ll-en-wasdel.gsm
ll-en-youhave.gsm
