#!/usr/bin/perl
# purpose of this script is to copy a broadcast message to valid boxes
# and set their new messages flag to 1
use Lifeline::DB;
use File::Copy;
use File::Path;
use Getopt::Std;
use strict;
my %opt;
getopts('ev',\%opt);
my $dbpat = "(coolaid|lifeline)";
my $msgpat = '\d+/messages/(\d+\.\d+\.gsm)$';
my $basedir = "/usr/local/asterisk";

print <<TXT unless $opt{e};
Syntax: %0 [-e][-v] {dbname} {message}
	Broadcast a message to all vm users
	-e actually copy message
	-v verbose
	{dbname} one of $dbpat
	{message} a gsm message in the form $msgpat
Doing a dry run:
TXT

my $db = shift;
die "need a valid db name! should be $dbpat." unless $db =~ /^$dbpat$/;
print "db $db\n" if $opt{v};
my $msgdir = "$basedir/$db-msgs";
die "no messages dir $msgdir!" unless -d $msgdir;

my $msg = shift;
die "bad message file name! should be like $msgpat." unless $msg =~ /$msgpat/;
my $msgfile = $1;
die "message $msgfile doesn't exist!" unless -f $msg;
print "msg $msg (msgfile $msgfile)\n" if $opt{v};


my $getq = "select box from $db.boxes where ((status = '' and paidto > current_date) or status = 'permanent')";
print "query: $getq\n" if $opt{v};
my $get = $ldb->prepare($getq);
my $updq = "update $db.boxes set new_msgs = 1 where box = ?";
my $upd = $ldb->prepare($updq);

$get->execute or die $get->errstr;
while (my $row = $get->fetch) {
	my ($box) = @$row;
	my $dir = "$msgdir/$box/messages";
	my $newfile = "$dir/$msgfile";
	unless (-d "$msgdir/$box/messages") {
		mkpath($dir,1) or die "can't make $dir: $!";
	} elsif (-f $newfile) {
		print "skipping: $newfile exists!\n";
	} elsif ($opt{e}) {
		copy $msg, $newfile or die "can't make $newfile: $!";
		$upd->execute($box) or die $upd->errstr;
	} else {
		print "will copy $msg to $newfile\n";
	}
}
$get->finish;

