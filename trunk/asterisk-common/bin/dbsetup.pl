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

print "building the numrange table\n";
for (my $i=0; $i<10_000; $i++) {
	$ldb->do("insert ignore into numrange (i) values ('$i')") or die $ldb->errstr;
}

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

print "Enter a super user user name: ";
my $suname = <STDIN>;
chomp $suname;
my $md5name = md5_hex($suname);
if ($names{$md5name} and !$opt{f}) {
	print "A super user with this name exists! Continue? [Y|n] ";
	my $yn = <STDIN>;
	exit if $yn =~ /^[Nn]/;
}

print "Enter password for superuser: ";
ReadMode('noecho');
my $supw = <STDIN>;
chomp $supw;
ReadMode(0);
print "\nRe-enter password: ";
ReadMode('noecho');
my $supwconf = <STDIN>;
chomp $supwconf;
ReadMode(0);

die "passwords don't match!" unless $supw eq $supwconf;

my $ins = $ldb->prepare(
	"replace into users (login,password,created,perms,notes) ".
	"values (md5(?),md5(?),now(),'s','super user')"
);
$ins->execute($suname,$supw) or die $ins->errstr;
print <<TXT;

Successfully created new super user.

You will want to create at least one vendor from the .../admin.php web page.
You can create invitation emails for new users with the .../emailsignup.php web page.

TXT

