#!/usr/bin/perl
use Asterisk::AGI;
my $cid = shift;
my $blocked = 0;
while (<DATA>) {
	chomp;
	$blocked = 1 and last if $cid eq $_;
}
my $agi = Asterisk::AGI->new;
$agi->set_variable('blocked',$blocked);

__DATA__
8663878751
8663446912
7783302273
7783289992
