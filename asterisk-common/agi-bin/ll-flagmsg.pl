#!/usr/bin/perl
# purpose of this script is to add an indicator that you have new messages
use Lifeline;
use LWP::Simple;
use File::stat;
use strict;
my $ll = Lifeline->init();
my $msg = shift;
$msg .= ".".$ll->{rectype};
my $msg_stat = stat($msg);
if (!defined $msg_stat or $msg_stat->size < $Lifeline::min_msg_size) {
	unlink $msg;
} else {
	$ll->flag_new_msgs(1);
	$ll->{new_msg} = $msg;
	$ll->log_calls('ll-flagmsg.pl',$ll->{last_page_result});
}
