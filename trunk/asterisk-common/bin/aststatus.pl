#!/usr/bin/perl
# get the asterisk status and save it to a file
# trim extra junk from the output
use strict;
my $astcmd = "/usr/local/asterisk/sbin/asterisk -rx 'core show uptime'"; 
my $astcache = "/usr/local/asterisk/html/tools/php/.asterisk-uptime";
my $astout = `$astcmd`;
$astout =~ s/.*(System.*seconds).*/$1/;
open CACHE, $astcache or die "can't open $astcache: $!";
print CACHE $astout;
close CACHE;


