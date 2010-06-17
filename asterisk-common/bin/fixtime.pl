#!/usr/bin/perl
# purpose of this script is to update the time on messages to the original time
use File::Find;
use strict;

my $start = "/usr/local/asterisk/lifeline-msgs";
find(\&wanted,$start);

sub wanted {
	return unless /(\d+)\.(\d+)\.gsm/;
	my $time = $1;
	my $ord = $2;
	print "changing time on $_ to ",scalar(localtime($time)),"\n";
#	utime $time,$time,$_;
}
