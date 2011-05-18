#!/usr/bin/perl
# purpose of this script is to check the login and then ferry the customer off to the main menu if successful
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need messages list
my $callerid = shift;
$ll->log_calls('ll-callstart.pl','',$callerid);

