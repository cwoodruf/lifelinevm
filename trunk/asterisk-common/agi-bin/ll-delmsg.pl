#!/usr/bin/perl
# purpose of this script is to delete a message resulting 
# from someone entering their security code while recording
use Lifeline;
use strict;
my $ll = Lifeline->init(); # use ->load if you need messages list
my $newmsg = shift;
$newmsg .= '.'.$ll->{rectype};
if (length($newmsg) and -f $newmsg and (stat($newmsg))[7] < $Lifeline::min_msg_size) {
	unlink($newmsg) or warn "can't unlink $newmsg: $!";
}
