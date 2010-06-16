#!/usr/bin/perl
# purpose of this script is to check the login and then ferry the customer off to the main menu if successful
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need messages list
my $seccode = shift;
$ll->check_login($seccode);
my $callerid = shift;
if ($ll->{login_ok}) {
	$ll->{agi}->set_priority(100);
	$ll->load_msgs;	
	$ll->play_msg_count('play "no messages" only');
#	$ll->log_calls('ll-login.pl','ok');
} else {
	$ll->log_calls('ll-login.pl','failed',$callerid);
}

