#!/usr/bin/perl
use Lifeline;
use strict;
my $ll = Lifeline->init(); 
my $event = shift;
my $tophone = shift;
$ll->log_free_calls($ll->get('SIPURI'), $event, $tophone);

