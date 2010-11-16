#!/usr/bin/perl
# purpose of this script is to find any email addresses for boxes that have new messages
# it then initiates the llremind.pl script on callbackpack.com to actually send the messages
use Lifeline::DB;
use strict;
my $ssh = '/usr/bin/ssh';
my $sendhost = 'callbackpack.com';
my $sendapp = '/vservers/callbackpack10/bin/llremind.pl -v';
my $query = 
	"select box,email,llphone,'lifeline.boxes' ".
	"from lifeline.boxes ".
	"where new_msgs=1 and reminder=0 ".
	"and email <> '' ".
	"and (paidto >= current_date() or status='permanent') ";
my $union =
	"union ".
	"select box,email,llphone,'coolaid.boxes' ".
	"from coolaid.boxes ".
	"where new_msgs=1 and reminder=0 ".
	"and email <> '' ".
	"and (paidto >= current_date() or status='permanent')";
print "$query\n";
my $get = $ldb->prepare($query.$union);

$get->execute or die $get->errstr;

my $send;
while (my $row = $get->fetch) {
	my ($box,$email,$llphone,$table) = @$row;
print "$box, $email, $llphone, $table\n";
	$ldb->do("update $table set reminder=1 where box='$box'") or die $ldb->errstr;
	warn "non-numeric box $box!" and next unless $box =~ /^\d+$/;
	warn "invalid email $email!" and next unless $email =~ /^\w[\w\.\-]*\@\w[\w\.\-]*\.\w{2,4}$/;
	$send .= "$box $email $llphone\n";
}
$get->finish;


open REMIND, "| $ssh $sendhost '$sendapp'" or die "can't open ssh to $sendhost: $!";
print REMIND $send;
close REMIND;

