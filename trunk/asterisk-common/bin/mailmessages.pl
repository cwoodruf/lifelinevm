#!/usr/bin/perl
# get all the messages for people who have confirmed emails and send them
use Lifeline::DB;
use Digest::SHA1 qw/sha1_hex/;
use strict;
my $mailmessage = "/usr/local/asterisk/bin/mailmessage.php";
my $get = $ldb->prepare(
	"select distinct boxes.box,email,llphone,message,seccode ".
	"from calls join boxes on (calls.box=boxes.box) ".
	"where emailconfirmed=1 ".
	"and calls.call_time >= current_date() ".
	"and calls.message <> '' ".
	"and calls.messagesent is null ".
	"order by boxes.box,call_time "
);
my $upd = $ldb->prepare(
	"update calls set messagesent=now() where box=? and message=?"
);
$get->execute or die $get->errstr;
while (my $row = $get->fetch) {
	my ($box,$email,$llphone,$message,$seccode) = @$row;
	my $key = sha1_hex($seccode.$box.$message);
	print "$box: $message\n";
	my $cmd = "$mailmessage '$llphone' '$box' '$email' '$message' '$key'";
	print "$cmd\n";
	system $cmd and die "can't email: $!";
}
