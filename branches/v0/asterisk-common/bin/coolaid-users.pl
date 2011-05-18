#!/usr/bin/perl
use Lifeline::DB;
use Data::Dumper;
use Getopt::Std;
our $salt;
do "/usr/local/asterisk/agi-bin/Lifeline/salt-coolaid";
use strict;
my %opt;
getopts('v',\%opt);

# fields:
my @fields = qw/
	BOX
	LANG
	SEC_CODE
	BOX_TYPE
	ADMIN_BOX
	NAME
	ADDRESS1
	ADDRESS2
	CITY
	PROV
	PHONE
	POST_CODE
	MISC
	TTL_PAID
	PAID_TO
	REMINDER
	START_DATE
	LAST_CALL
	CALLS
	MESSAGES
	ACTIVE
	BOXDAYS
	BDUPDATE
	BDs_OWED
	BD_VALUE
/;

my @addressfields = qw/
	ADDRESS1
	ADDRESS2
	CITY
	PROV
	POST_CODE
/;

# removed PAID_TO,ACTIVE 2010-08-11
my @interesting = qw/
	BOX
	SEC_CODE
	NAME
	MISC
	PHONE
	PAID_TO
	ACTIVE
/;

=filter out
# needed right now for coolaid 
my $existing = $ldb->selectall_arrayref("select box from coolaid.boxes");
my %exists;
if (ref $existing ne 'ARRAY') {
	die "can't grab list of existing boxes!: ".$ldb->errstr;
} else {
	%exists = map { $_->[0],1 } @$existing;
}
=cut

my $ins = $ldb->prepare(
	"replace into coolaid.boxes set box=?,seccode=md5(?),vid=6971,name=?,notes=?,llphone=?,paidto=?,status=?"
);
$coolaiddatafile = shift || $coolaiddatafile;
open IN, $coolaiddatafile or die "can't open $coolaiddatafile: $!";
my %month = (
	Jan => 1,
	Feb => 2,
	Mar => 3,
	Apr => 4,
	May => 5,
	Jun => 6,
	Jul => 7,
	Aug => 8,
	Sep => 9,
	Oct => 10,
	Nov => 11,
	Dec => 12,
);
while (my $raw = <IN>) {
	print $raw if $opt{v};
	chomp $raw;
	my %in;
	@in{@fields} = split ",", $raw;

	$in{ACTIVE} = $in{ACTIVE} eq 'INACTIVE' ? 'deleted': '';
	if ($in{PAID_TO} eq '' and $in{ACTIVE} ne 'deleted') {
		$in{ACTIVE} = 'permanent';
	} else {
		$in{PAID_TO} =~ s/(\d+)-(\w+)-(\d+)/sprintf("%04d-%02d-%02d",$3+2000,$month{$2},$1)/e;
	}

	if ($in{BOX_TYPE} =~ /MSG/) {
		$in{SEC_CODE} .= $salt;
		$in{PHONE} = '250 383-5144';
		print Dumper([@in{@interesting}]) if $opt{v};
		$ins->execute(@in{@interesting}) or die $ins->errstr;
	}
}

