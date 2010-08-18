#!/usr/bin/perl
# purpose of this script is to check the registration for sip channels and attempt a restart
# if registration has failed
use Getopt::Std;
use strict;
my %opt;
getopts('v', \%opt);

my $asterisk = '/usr/local/asterisk/sbin/asterisk ';
my $showcmd = " -rx 'sip show registry'";
my $restartcmd = " -rx 'restart when convenient'";

my $notfailed = 'Registered';
my $sipregs = `$asterisk $showcmd`;
print "sip registry:\n$sipregs" if $opt{v};

=filter out
Host                            Username       Refresh State                Reg.Time                 
sip-slb.voicemeup.com:5060      deravoicemeu       105 Registered           Wed, 18 Aug 2010 06:45:36
sip.ca2.link2voip.com:5060      deravm             105 Registered           Wed, 18 Aug 2010 06:46:12
sip.ca1.link2voip.com:5060      deravm             105 Registered           Wed, 18 Aug 2010 06:46:33
=cut

my $restart = 0;
my $count = 0;
foreach my $reg (split "\n", $sipregs) {
	my @everything = my ($host, $user, $refresh, $state,@time)  = split /\s+/, $reg;
	next if $host =~ /^(?:Host|Unable)$/;
	if ($state ne $notfailed) {
		print STDERR "bad sip registration for $user\@$host: $state (@time)\n";
		$restart = 1;
	}
	$count++;
}

if ($restart) {
	my $cmd = "$asterisk $restartcmd";
	print "doing: $cmd\n" if $opt{v};
	system $cmd and warn "problem restarting asterisk: $!";
} elsif ($count == 0) {
	print "doing: $asterisk\n" if $opt{v};
	system $asterisk and warn "can't start asterisk: $!";
} else {
	print "not doing anything\n" if $opt{v};
}

