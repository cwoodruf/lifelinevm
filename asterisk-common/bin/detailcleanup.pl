#!/usr/bin/perl
# purpose of this script is to remove gsm.txt files when there are no .gsm or .deleted.gsm files
use strict;
my @dirs = qw{../lifeline-msgs ../coolaid-msgs};
foreach my $dir (@dirs) {
	my @txts = glob("$dir/*/*/*.gsm.txt");
	foreach my $txt (@txts) {
		my ($base,$ext) = ($txt=~m#(.*)(\.gsm\.txt)#);
		unless (-f "$base.wav" or -f "$base.gsm" or -f "$base.deleted.gsm") {
			print "no message for $txt\n";
			unlink $txt or warn "can't unlink: $!";
		}
	}
}
