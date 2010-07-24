#!/usr/bin/perl
# purpose of this script is to get a flag for the voice mailbox and make it available for the dialplan
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need messages list
my $flag = shift;
# this will silently fail if flag doesn't exist or box is blank
$ll->getflag($flag);
