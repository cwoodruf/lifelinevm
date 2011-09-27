#!/usr/bin/perl
# purpose of this script is to get an initial superuser for the db

use lib qw{/usr/local/asterisk/var/lib/asterisk/agi-bin};
use Lifeline::DB;
use Term::ReadKey;
use Getopt::Std;
use Digest::MD5 qw/md5_hex/;
use strict;
my (%opt,%names);
getopts('fh',\%opt);

if ($opt{h}) {
	print <<TXT;
$0 [-f|-h]
	Builds the numrange table and creates a new super user.
	Intended for a new installation of the lifeline website.
	-f force create of new super user
	-h this help
TXT
}

print "Enter database name you'd like to set up with super user and root user: ";
my $dbname = <STDIN>;
chomp $dbname;
my $checkdb = $ldb->prepare("select i from $dbname.numrange");
eval {
	$checkdb->execute or die $checkdb->errstr;
};
print "\n\ninvalid db name - did you create this db yet? See asterisk-common/README for more info.\n" and exit
	if $@;

unless ($checkdb->rows) {
	print "building the numrange table\n";
	for (my $i=0; $i<10_000; $i++) {
		$ldb->do("insert ignore into numrange (i) values ('$i')") or die $ldb->errstr;
	}
}
$checkdb->finish;

print "looking for existing super users\n";
my $getsu = $ldb->prepare("select * from users where perms = 's'");
$getsu->execute or die $getsu->errstr;
my %md5names;
if ($getsu->rows) {
	while (my $row = $getsu->fetchrow_hashref) {
		$names{$row->{login}}++;
	}
	$getsu->finish;
	print "Superusers already exist for this installation. ";
	unless ($opt{f}) {
		print "Continue? [y|N] ";
		my $yn = <STDIN>;
		exit unless $yn =~ /^[yY]/;
	} else {
		print "\n";
	}
}
print "This user is only used to create emailable links for new logins at the moment ...\n";
userset('super');
print "The root user is the main user for administering the voice mail system.\n";
userset('root');

print <<TXT;

Successfully created new super user and root user.

You will want to update the root vendor information in the .../admin.php page.

Use the superuser login to make invitation emails for new users with the .../emailsignup.php web page.

TXT

sub userset {
	my ($type) = @_;
	die "don't know type $type!" unless $type =~ /super|root/;

	print "Enter a $type user user name: ";
	my $suname = <STDIN>;
	chomp $suname;
	my $md5name = md5_hex($suname);
	if ($names{$md5name} and !$opt{f}) {
		print "A $type user with this name exists! Continue? [Y|n] ";
		my $yn = <STDIN>;
		exit if $yn =~ /^[Nn]/;
	}

	print "Enter password for $type user: ";
	ReadMode('noecho');
	my $supw = <STDIN>;
	chomp $supw;
	ReadMode(0);
	print "\nRe-enter password: ";
	ReadMode('noecho');
	my $supwconf = <STDIN>;
	chomp $supwconf;
	ReadMode(0);
	print "\n";

	die "passwords don't match!" unless $supw eq $supwconf;

	my ($md5name, $md5pw, $vid, $perms);
	if ($type eq 'root') {
		print "Enter vendor name for root user: ";
		my $vendor = <STDIN>;
		chomp $vendor;
		my $insv = $ldb->prepare(
			"insert into vendors (vendor,acctype) values (?,'login')"
		);
		$insv->execute($vendor) or die $insv->errstr;
		my $getvid = $ldb->selectrow_arrayref("select last_insert_id() from vendors") or die $ldb->errstr;
		$vid = $getvid->[0];
		print "created vendor $vid\n";
		$ldb->do("update vendors set parent=vid where parent is null") or die $ldb->errstr;
		$perms = 'edit:boxes:invoices:logins:vendors';
		$md5name = $suname;
		
	} elsif ($type eq 'super') {
		$vid = 0;
		$perms = 's';
		$md5name = md5_hex($suname);
	}
	my $ins = $ldb->prepare(
		"replace into users (login,password,created,vid,perms,notes) ".
		"values (?,md5(?),now(),?,?,'$type user')"
	);
	$ins->execute($md5name,$supw,$vid,$perms) or die $ins->errstr;
	print "created $type user\n";
}

