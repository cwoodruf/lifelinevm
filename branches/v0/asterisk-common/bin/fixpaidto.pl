#!/usr/bin/perl
# purpose of this script is to update paidto dates on the new system with
# later paidto dates from the previous system
use Data::Dumper;
use Lifeline::DB;
our $salt;
do "/usr/local/asterisk/agi-bin/Lifeline/salt";
use strict;

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

my $get = $ldb->prepare("select paidto from boxes where box=?");
my $upd = $ldb->prepare(
        "insert into boxes (box,seccode,vid,name,notes,llphone) values (?,md5(?),?,?,?,?)"
);

my @interesting = qw/
        BOX
        SEC_CODE
        ADMIN_BOX
        NAME
        MISC
        PHONE
/;

while (<>) {
        chomp;
        my %in; @in{@fields} = split ",";
        (my $paidto = $in{PAID_TO}) =~ s/(\d+)-(\w+)-(\d+)/sprintf("%04d-%02d-%02d",$3+2000,$month{$2},$1)/e;
	next unless $paidto =~ /^\d\d\d\d-\d\d-\d\d$/;
	print "checking $in{BOX} paid to $in{PAID_TO} $paidto\n";
	$get->execute($in{BOX}) or die $get->errstr;
	if ($get->rows > 0) {
		my $row = $get->fetchrow_hashref;
		if ($paidto gt $row->{paidto}) {
			print "updating $row->{paidto} with $paidto\n";
			$ldb->do("update boxes set paidto='$paidto' where box='$in{BOX}'") 
				or die "can't update paidto: ".$ldb->errstr;
		} else {
			print "db paidto $row->{paidto} gte $paidto\n";
		}
	} else {
		print " no record found adding!\n";
                $in{SEC_CODE} .= $salt;
                $in{PHONE} = '604 682-3269';
		$in{PAID_TO} = $paidto;
                # the only boxes we are interested in at this point already exist
                $upd->execute(@in{@interesting}) or die $upd->errstr;
	}
	$get->finish;
}

