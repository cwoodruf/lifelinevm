#!/usr/bin/perl
# purpose of this script is to grab an extraneous data from the calls log table on the other servers
# as there is no primary key we have to be careful about not duplicating data
# this should be only run on lifelinevm.net
$mainhost = 'lifelinevm.net';
die "should only run on $mainhost not $ENV{HOSTNAME}!" unless $ENV{HOSTNAME} eq $mainhost;

use Lifeline::DB;
use Getopt::Std;
use Data::Dumper;
use strict;
my %opt;
getopts('v',\%opt);

my $lastsync;
my $lastupdate = $ldb->selectrow_arrayref(
	"select max(synctime) from sync_calls"
);
if ('ARRAY' eq ref $lastupdate) {
	$lastsync = $lastupdate->[0]; 
}

=filter out
mysql> desc calls;
+-----------+-------------+------+-----+-------------------+-------+
| Field     | Type        | Null | Key | Default           | Extra |
+-----------+-------------+------+-----+-------------------+-------+
| box       | varchar(32) | NO   | PRI |                   |       | 
| call_time | timestamp   | NO   | PRI | CURRENT_TIMESTAMP |       | 
| action    | varchar(32) | NO   | PRI |                   |       | 
| status    | varchar(32) | NO   | PRI |                   |       | 
| message   | text        | NO   | PRI | NULL              |       | 
| callerid  | varchar(64) | YES  |     | NULL              |       | 
| vid       | int(11)     | YES  |     | NULL              |       | 
+-----------+-------------+------+-----+-------------------+-------+
=cut

my @fields = qw/box call_time action status message callerid vid/;
my $fieldlist = join ",", @fields;
my $qmarks = join ",", map { '?' } @fields;
my $updateq = "insert ignore into calls ($fieldlist) values ($qmarks)";
my $update = $ldb->prepare($updateq);

foreach (@Lifeline::DB::foreign) {
	print "host $_->{host}\n" if $opt{v};
	my $dsn = Lifeline::DB::mkdsn($_->{host},$_->{port});
	print "dsn $dsn\n" if $opt{v};
	my $dbh = Lifeline::DB::mkldb($dsn,$_->{user},$_->{password});
	
	my $getq = "select * from calls"; 
	if (defined $lastsync) {
		$getq .= " where call_time > '$lastsync'";
	}
	my $get = $dbh->prepare($getq);
	$get->execute or die $get->errstr;
	while (my $row = $get->fetchrow_hashref) {
		print join ",", values %$row,"\n" if $opt{v};
		$update->execute(@$row{@fields}) or die $update->errstr;
	}
	$get->finish;
}

$ldb->do("update sync_calls set synctime=now()") or die $ldb->errstr;
