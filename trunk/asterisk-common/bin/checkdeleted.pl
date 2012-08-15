#!/usr/bin/perl
# purpose of this script is to get all deleted boxes from the db and 
# check that their messages are deleted
use Lifeline::DB;
use File::Find;
use File::Copy;
use Getopt::Std;
use strict;
my %opt;
getopts("vd",\%opt);
print "use -d to hide messages automatically, -v verbose\n" if $opt{v};

my $who = shift;
chomp $who;
my $pat = qr/^(coolaid|lifeline)$/;
die "$who should be $pat" unless $who =~ $pat;
my $basepath = "/usr/local/asterisk/$who-msgs/";

my $get = $ldb->prepare(
	"select box,status,paidto from $who.boxes ".
	"where status = 'deleted' or status like 'add % months' or datediff(now(),paidto) > 90 ".
	"order by box "
);
$get->execute or die $get->errstr;

while (my $row = $get->fetch) {
	my ($box, $status, $paidto) = @$row;
	my $srchdir = "$basepath/$box";
	&unlink_listen($srchdir);
	print scalar(localtime),": checking $box (status $status, paidto $paidto) $srchdir\n" if $opt{v};
	warn "$srchdir not a directory!" unless $srchdir;
	finddepth(\&wanted, $srchdir);
}

$get->finish;

sub wanted {
	return unless /\.gsm$/;
	unless (/deleted\.gsm$/) {
		print scalar(localtime),": NOT DELETED: $File::Find::name\n";
		if ($opt{d} and $_ !~ /deleted\.gsm$/) {
			print scalar(localtime),": hiding message $File::Find::name\n";
			(my $new = $_) =~ s/(.*)(\.gsm)/$1.deleted$2/;
			move $_, $new or die "can't move $_ to $new: $!";
		}
	}
}

sub unlink_listen {
	my ($srchdir) = @_;
	return unless -d "$srchdir/listen";
	foreach my $f (glob("$srchdir/listen/*.*")) {
		unlink $f;
	}
}

