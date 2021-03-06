#!/usr/bin/perl
# purpose of this script is to find any email addresses for boxes that have new messages
# it then initiates the llremind.pl script on callbackpack.com to actually send the messages
use Lifeline::DB;
use Digest::SHA1 qw/sha1_hex/;
use File::Copy;
use File::Path;
use strict;
my $sox = '/usr/bin/sox';
my $listendir = "/usr/local/asterisk/html/listen";
my $basedir = "/usr/local/asterisk";
my $ssh = '/usr/bin/ssh';
my $sendhost = 'callbackpack.com';
my $sendapp = '/vservers/callbackpack10/bin/llremind.pl -v';
my $condition = 
	"email <> '' and reminder=0 and (".
	"(new_msgs=1 and (paidto >= current_date() or status = 'permanent')) ".
	"or (status = '' and paidto >= current_date() - interval 3 week and paidto <= current_date()))";

my $query = 
	"select box,email,llphone,'lifeline.boxes','lifeline-msgs',seccode,paidto,new_msgs ".
	"from lifeline.boxes ".
	"where $condition ";
my $union =
	"union ".
	"select box,email,llphone,'coolaid.boxes','coolaid-msgs',seccode,paidto,new_msgs ".
	"from coolaid.boxes ".
	"where $condition ";
print "$query $union\n";
my $get = $ldb->prepare($query.$union);

$get->execute or die $get->errstr;

my $send;
while (my $row = $get->fetch) {
	my ($box,$email,$llphone,$table,$dir,$seccode,$paidto,$newmsgs) = @$row;
	my $key = sha1_hex($box.$email.$seccode);
	my $boxdir = "$basedir/$dir/$box";
	my $msgdir = "$boxdir/messages";
	my $boxlistendir = "$boxdir/listen";
	mkpath $boxlistendir unless -d $boxlistendir;
	symlink $boxlistendir, "$listendir/$key" unless -e "$listendir/$key";
	&mkwavs($msgdir,$boxlistendir);
	$ldb->do("update $table set reminder=1,listenkey='$key' where box='$box'") or die $ldb->errstr;
	warn "non-numeric box $box!" and next unless $box =~ /^\d+$/;
	warn "invalid email $email!" and next unless $email =~ /^\w[\w\.\-\+\=]*\@\w[\w\.\-]*\.\w{2,4}$/;
	$send .= "$box $email $key $paidto $newmsgs $llphone\n";
}
$get->finish;

sub mkwavs {
	my ($msgdir,$path) = @_;
	my @msgs = glob("$msgdir/*.gsm");
	foreach my $msg (@msgs) {
		my ($base, $ext) = ($msg=~m#(.*)(\.gsm)#);
		next if $base =~ /deleted$/;
		(my $listenbase = $base) =~ s#/messages/#/listen/#;
		my $cmd = "$sox '$msg' '$listenbase.wav'";
		system $cmd unless -f "$listenbase.wav";
		copy "$base.gsm.txt", "$path";
	}
}

open REMIND, "| $ssh $sendhost '$sendapp'" or die "can't open ssh to $sendhost: $!";
print REMIND $send;
close REMIND;

