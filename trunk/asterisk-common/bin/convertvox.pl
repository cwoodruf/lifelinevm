#!/usr/bin/perl
# purpose of this script is to read the dialogic lifeline files and convert them to gsm and save them
use strict;
use File::Path;
use Lifeline::DB;

my $inrate = 6000; # dialogic vox sample rate for input files
my $outrate = 8000; # gsm sample rate

my $source = shift;
$source = "/usr/local/asterisk/Lifeline" if $source eq '';
die "source $source is not a directory!" unless -d $source;
my $target = shift;
$target = "/usr/local/asterisk/lifeline-msgs" if $target eq '';
die "target $target is not a directory!" unless -d $target;

my $delbox = ll_deleted_boxes();

opendir SRC, $source or die "can't open directory $source: $!";

while (my $item = readdir(SRC)) { 
	next unless -f "$source/$item";
next unless $item =~ /^513/;
	next unless $item =~ m#(\d{4})(\d*)\.(MSG|GRT)$#;
	my ($box,$msg,$type) = ($1,$2,$3);
	if ($delbox->{$box}) {
		print "box $box deleted ignoring $item\n";
		next;
	}
	my $infile = "$source/$item";
	$box = "$target/$box";
	mkpath("$box/messages") or die "can't make $box/messages: $!" unless -d "$box/messages";
	my @stat = stat($infile);
	my $outfile;
	if ($type eq 'MSG') {
		$outfile = sprintf("$box/messages/%d.%d.gsm", $stat[9], $msg);
	} elsif ($type eq 'GRT') {
		$outfile = "$box/greeting.gsm";
	} else {
		warn "don't understand type $type for $item!";
		next;
	}
	next if -f $outfile;
	my @check = glob("$box/messages/[0-9]*.$msg.gsm");
	if (scalar (@check)) {
		foreach my $oldmsg (@check) {
			my $msgbase = ($oldmsg=~m#(.*)\.gsm$#)[0];
			print "rename $oldmsg to $msgbase.deleted.gsm\n";
			rename $oldmsg, "$msgbase.deleted.gsm";
		}
	}
	print "$infile -> $outfile\n";
	my $cmd = "sox -v .7 -t .vox -r $inrate $infile -r $outrate $outfile";
	print "$cmd\n"; 
	system $cmd and die "can't convert $infile to $outfile: $!";
}
closedir SRC;
