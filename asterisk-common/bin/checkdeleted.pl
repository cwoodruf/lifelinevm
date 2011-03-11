#!/usr/bin/perl
# purpose of this script is to get all deleted boxes from the db and 
# check that their messages are deleted
use Lifeline::DB;
use File::Find;
use Getopt::Std;
use strict;
my %opt;
getopts("v",\%opt);

my $who = shift;
chomp $who;
my $pat = qr/^(coolaid|lifeline)$/;
die "$who should be $pat" unless $who =~ $pat;
my $basepath = "/usr/local/asterisk/$who-msgs/";

my $get = $ldb->prepare("select box from $who.boxes where status = 'deleted'");
$get->execute or die $get->errstr;

while (my $row = $get->fetch) {
	my ($box) = @$row;
	my $srchdir = "$basepath/$box";
	print "checking $srchdir\n" if $opt{v};
	warn "$srchdir not a directory!" unless $srchdir;
	finddepth(\&wanted, $srchdir);
}

$get->finish;

sub wanted {
	return unless /\.gsm$/;
	print "NOT DELETED: $File::Find::name\n" unless /deleted\.gsm$/;
}

