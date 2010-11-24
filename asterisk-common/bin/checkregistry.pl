#!/usr/bin/perl
# purpose of this script is to check the registration for sip channels and attempt a restart
# if registration has failed
use Getopt::Std;
use strict;
my %opt;
# options: v: verbose, m: send mail on restart
getopts('vm:', \%opt);

my $asterisk = '/usr/local/asterisk/sbin/asterisk ';
my $showcmd = " -rnx 'sip show registry'";
#my $restartcmd = " -rnx 'restart when convenient'";
my $restartcmd = " -rnx 'sip reload'";
my $mail = '/usr/bin/mail';
my $mailto = $opt{m}; 

my $notfailed = 'Registered';
my $sipregs = `$asterisk $showcmd`;
print "sip registry:\n$sipregs" if $opt{v};

=filter out
1.4
Host                            Username       Refresh State                Reg.Time                 
sip-slb.voicemeup.com:5060      deravoicemeu       105 Registered           Wed, 18 Aug 2010 06:45:36
sip.ca2.link2voip.com:5060      deravm             105 Registered           Wed, 18 Aug 2010 06:46:12
sip.ca1.link2voip.com:5060      deravm             105 Registered           Wed, 18 Aug 2010 06:46:33
1.6
Host                           dnsmgr Username       Refresh State                Reg.Time
sip-slb.voicemeup.com:5060     N      deravmumain        105 Registered           Sat, 20 Nov 2010 06:20:01
sip.ca2.link2voip.com:5060     N      lifelineback       105 Registered           Sat, 20 Nov 2010 06:18:18
sip.ca1.link2voip.com:5060     N      lifelineback       105 Registered           Sat, 20 Nov 2010 06:18:18
3 SIP registrations.
=cut

my $restart = 0;
my $count = 0;
my ($host, $dnsmgr, $user, $refresh, $state,@time);
my $version = (($sipregs =~ m#dnsmgr#) ? '1.6' : '1.4');
print "version $version\n" if $opt{v};
foreach my $reg (split "\n", $sipregs) {
	if ($version eq '1.6') {
		($host, $dnsmgr, $user, $refresh, $state,@time)  = split /\s+/, $reg;
		next if $host =~ /^(?:Host|Unable|\d+)$/;
	} else {
		($host, $user, $refresh, $state,@time)  = split /\s+/, $reg;
		next if $host =~ /^(?:Host|Unable)$/;
	}
	if ($state ne $notfailed) {
		print STDERR "bad sip registration for $user\@$host: $state (@time)\n";
		$restart = 1;
	}
	$count++;
}

if ($restart) {
	my $cmd = "$asterisk $restartcmd";
	print "doing: $cmd\n" if $opt{v};
	system qq/echo "doing $cmd" | $mail -s "lifelinevm.net sip restart" $mailto/
		if $opt{m};
	system $cmd and warn "problem restarting asterisk: $!";
} elsif ($count == 0) {
	print "doing: $asterisk\n" if $opt{v};
	system qq/echo "doing $asterisk" | $mail -s "lifelinevm.net asterisk restart" $mailto/
		if $opt{m};
	system $asterisk and warn "can't start asterisk: $!";
} else {
	print "not doing anything\n" if $opt{v};
}

