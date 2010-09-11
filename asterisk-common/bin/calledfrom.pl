#!/usr/bin/perl
# purpose of this script is to determine which external phone numbers are used by most people to call in for messages
use Lifeline::DB;
use strict;

my $get = $ldb->prepare(
	"select distinct substr(callerid,-11,10) as phone, box ".
	"from calls ".
	# 2010-08-27 is the first day all calls were going through voicemeup
	"where action='ll-login.pl' and callstart > unix_timestamp('2010-08-27')"
);

$get->execute or die $get->errstr;
my (%phones);
while (my $row = $get->fetch) {
	my ($phone,$box) = @$row;
	$phones{$phone}++;
}
$get->finish;

my $threshold = 3;
foreach (sort { $phones{$a} <=> $phones{$b} } keys %phones) {
	next unless $phones{$_} > $threshold; 
	print "$_, $phones{$_}\n";
}
