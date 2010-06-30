#!/usr/bin/perl
use File::Find;
use Lifeline::DB;
use strict;

finddepth(\&wanted, $llmsgdir);
sub wanted {
	next unless /(\d+\.\d+)\.deleted\.gsm/;
	my $stub = $1;
	if (-f "$stub.gsm") {
		print "found duplicate of $_\n";
		unlink "$stub.gsm" or die "can't unlink: $!";
	}
}

