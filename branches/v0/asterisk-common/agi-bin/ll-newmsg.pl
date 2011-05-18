#!/usr/bin/perl
# purpose of this script is to create a new message name
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need messages list
$ll->chkdirs;
$ll->set('newmsg',$ll->newmsg);
# $ll->log_calls('ll-newmsg.pl');
