#!/usr/bin/perl
use Lifeline::DB;
use Data::Dumper;
use Getopt::Std;
our $salt;
do "/usr/local/asterisk/agi-bin/Lifeline/salt";
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
	ADMIN_BOX
	NAME
	MISC
	PHONE
/;

=filter out
# not needed right now as we are only doing updates
my $existing = $ldb->selectall_arrayref("select box from boxes");
my %exists;
if (ref $existing ne 'ARRAY') {
	die "can't grab list of existing boxes!: ".$ldb->errstr;
} else {
	%exists = map { $_->[0],1 } @$existing;
}
=cut

my $upd = $ldb->prepare(
	"update boxes set box=?,seccode=md5(?),vid=?,name=?,notes=?,llphone=? where box=?"
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
		);
		print Dumper(\%admin) if $opt{v};

		my $rows = $ldb->do(
			"insert ignore into vendors ".
			"(".(join ",", keys %admin).") values ".
			"(".(join ",", map { $ldb->quote($_); } values %admin).")"
		) or die $ldb->errstr;

	} elsif ($in{BOX_TYPE} =~ /MSG/) {
		$in{SEC_CODE} .= $salt;
		$in{PHONE} = '604 682-3269';
		print Dumper([@in{@interesting}]) if $opt{v};
		# the only boxes we are interested in at this point already exist
		$upd->execute(@in{@interesting},$in{BOX}) or die $upd->errstr;
	}
}

