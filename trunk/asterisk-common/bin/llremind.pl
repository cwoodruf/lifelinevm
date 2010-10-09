#!/usr/bin/perl
# purpose of this script is to send email reminders to voice mail subscribers when they get new messages
use Getopt::Std;
use Digest::MD5 qw/md5_hex/;
use strict;
my %opt;
getopts('v',\%opt);

my $mail = "/usr/bin/mail";
my $from = 'noreply@callbackpack.com';
my $optoutdir = '/vservers/callbackpack10/bin/llremind/optout';

my %emails;
while (<>) {
	chomp;
	my ($box, $email, @llphone) = split;
	my $llphone = join " ", @llphone;
	next unless $box =~ /^\d+$/;
	next unless $email =~ /^\w[\w\-\.]*\@\w[\w\-\.]*\.\w{2,4}$/;
	next unless $llphone =~ /^[\w\-\.\(\) ]{0,20}$/;
	push @{$emails{$email}}, [$llphone, $box];
}

foreach my $email (keys %emails) {
	foreach my $edata (@{$emails{$email}}) {
		my ($llphone,$box) = @$edata;
		print "$email has opted out\n" and next if -f "$optoutdir/".md5_hex($email); 
		
		my $vphone = "$llphone ext $box";
		print "sending reminders for $vphone to $email\n" if $opt{v};
		open MAIL, "| $mail -a 'from: $from' -s 'new voice mail for $vphone' $email" 
			or die "can't open $mail: $!";
		print MAIL <<TXT;
DO NOT REPLY TO THIS EMAIL MESSAGE.

You have new voice mail messages in $vphone.

Lifeline Voice Mail
http://callbackpack.com
604 682-3269 Ext 6040

Opt out: http://callbackpack.com/optout.php

TXT
		close MAIL;
		sleep(2);
	}
}

