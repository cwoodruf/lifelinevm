#!/usr/bin/perl
# purpose of this script is to add an indicator that you have new messages
use Lifeline;
use File::stat;
use strict;
my $ll = Lifeline->init();
my $msg = shift;
$msg .= ".".$ll->{rectype};
my $callerid = shift;
my $msgstart = shift;
my $duration = time - $msgstart;
if ($duration > $Lifeline::MINDURATION) {
	# $ll->flag_new_msgs(1);
	$ll->setflag('new_msgs',1);
	$ll->setflag('reminder',0);
	$ll->{new_msg} = $msg;
	$ll->log_calls('ll-flagmsg.pl',$ll->{last_page_result},$callerid);
} else {
	unlink $msg or warn "can't unlink message $msg: $!";
}

