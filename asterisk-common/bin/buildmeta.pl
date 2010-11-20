#!/usr/bin/perl
# rebuild the meta.pl files for all boxes
use Lifeline::DB;
use Data::Dumper;
use File::Path;
use strict;
my $dbname = shift;
die "need a db name!" if $dbname eq '';

my $msgdir = shift;
die "need a message directory!" unless -d $msgdir;

my $getboxes = $ldb->prepare("select box from $dbname.boxes where status not like '%deleted%'");
$getboxes->execute or die $getboxes->errstr;
while (my $row = $getboxes->fetch) {
	my ($box) = @$row;
	&findbox($box);
}
$getboxes->finish;

# based on the findbox method in Lifeline
sub findbox {
	my $box = shift;
	warn "invalid box $box" and return unless $box =~ /^\d+$/;
	my $boxdir = "$msgdir/$box";

	mkpath($boxdir) or warn "can't make $boxdir: $!" and return unless -d $boxdir;
	my $meta = "$boxdir/meta.pl";

	my $exists = $ldb->selectrow_arrayref(
		"select seccode,UNIX_TIMESTAMP(paidto),new_msgs,email,status,vid,announcement ".
		"from boxes where box='$box' "
	);
	if (open CACHE, "> $meta") { 
		print CACHE Data::Dumper->Dump([$exists],[qw/exists/]);
		close CACHE;
	} else {
		warn "can't open $meta: aborting!";
	}
}

