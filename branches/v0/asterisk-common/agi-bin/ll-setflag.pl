#!/usr/bin/perl
# purpose of this script is to set a flag
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need messages list
my $flag = shift;
my $value = shift;
# this will silently fail if flag doesn't exist or box is blank
$ll->setflag($flag, $value);
