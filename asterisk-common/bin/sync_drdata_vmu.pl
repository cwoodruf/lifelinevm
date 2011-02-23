#!/usr/bin/perl
# purpose of this script is to sync data from vmu call data with our local db
use Lifeline::DB;
use Getopt::Std;
my %opt;
getopts('v',\%opt);
our $username;
our $password;
no strict;
do '/home/asterisk/agi/Lifeline/database';
use strict;
my $ddb = Lifeline::DB::drdatadb();
my $ldb = Lifeline::DB::mkdbh('lifeline');

my $latest = $ldb->selectrow_arrayref(
	"select max(date) from voicemeupcalls"
);
die "error getting date!" unless $latest->[0] =~ /^\d\d\d\d-\d\d-\d\d/;

print scalar(localtime),": getting data from $latest->[0] onwards\n";

my $get = $ddb->prepare("select * from voicemeupcalls where date >= ?");
$get->execute($latest->[0]) or die $get->errstr;

while (my $row = $get->fetchrow_hashref) {
	my @fields;
	foreach (keys %$row) {
		push @fields, "$_=".$ldb->quote($row->{$_});
	}
	my $query = "insert ignore into voicemeupcalls set ".(join ',', @fields);
	print $query,"\n" if $opt{v};
	$ldb->do($query) or die "$query: ".$ldb->errstr;
}

$get->finish;

