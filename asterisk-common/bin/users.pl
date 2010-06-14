#!/usr/bin/perl
# purpose of this script is to scan the users.csv backup created by lifeline's (DOS) backup and 
# import it into the lifeline (asterisk) database
use Lifeline::DB;
use Data::Dumper;
use Getopt::Std;
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

my @interesting = qw/
	BOX
	SEC_CODE
	ADMIN_BOX
	NAME
	MISC
	ACTIVE
	PAID_TO
/;

my $ins = $ldb->prepare(
	"replace into boxes (box,seccode,vid,name,notes,status,paidto) ".
	"values (?,md5(?),?,?,?,?,?) "
);

open IN, $lldatafile or die "can't open $lldatafile: $!";
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

	$in{PAID_TO} =~ s/(\d+)-(\w+)-(\d+)/sprintf("%04d-%02d-%02d",$3+2000,$month{$2},$1)/e;
	$in{ACTIVE} = $in{ACTIVE} eq 'INACTIVE' ? 'deleted': '';

	if ($in{BOX_TYPE} eq 'PAYC') {
		my %admin = (
			'vid' => $in{BOX},
			'vendor' => $in{NAME},
			'address' => (join " ", @in{@addressfields}),
			'phone' => $in{PHONE},
			'notes' => $in{MISC},
		);
		print Dumper(\%admin) if $opt{v};

		my $rows = $ldb->do(
			"insert ignore into vendors ".
			"(".(join ",", keys %admin).") values ".
			"(".(join ",", map { $ldb->quote($_); } values %admin).")"
		) or die $ldb->errstr;

	} elsif ($in{BOX_TYPE} =~ /MSG/) {
		print Dumper([@in{@interesting}]) if $opt{v};
		$ins->execute(@in{@interesting}) or die $ins->errstr;
	}
}

