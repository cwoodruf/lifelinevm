#!/usr/bin/perl
# purpose of this script is to play the number of messages for a voicemail box
use Lifeline;
use strict;

# initialize lifeline
my $ll = Lifeline->init();
$ll->play_msg_count();
