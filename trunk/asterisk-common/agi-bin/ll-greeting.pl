#!/usr/bin/perl
# purpose of this script is to load the greeting variable with the correct greeting file
use Lifeline;
use strict;
my $ll = Lifeline->init(refresh => 1);
$ll->set('greeting',$ll->greeting);
$ll->set('saybox',$ll->saybox);
