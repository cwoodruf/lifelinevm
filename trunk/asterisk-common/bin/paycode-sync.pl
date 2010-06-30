#!/usr/bin/perl
# uses the pcavail data from the website to update number of months in the new vm system
use Data::Dumper; 
use Lifeline::DB;
use Getopt::Std;
use strict;
my %opt;
getopts('v',\%opt);

my $pcdir = "/usr/local/asterisk/paycode";

my $pcavail = "$pcdir/pcavail.csv";
open PC, "< $pcavail" or die "can't open $pcavail: $!";

my (%counts,%vendors);
while (my $line = <PC>) {
	chomp $line;
	my @F = split ',', $line;
	next unless $F[0] =~ /\d/ or $F[1] =~ /JobsNOW|JobStores/; 
	$counts{$F[1]}{$F[0]} += $F[3]; 
	$counts{$F[1]}{ttl} += $F[3]; 
}
close PC;

# grab active customers and remove any old test related stuff
my @active = grep !/CALM|Brooks|Lily|ZZdummy|ZTest|Johnson|Misc/, sort keys %counts;

my $custlist = "$pcdir/custlist.csv";
open CUST, "< $custlist" or die "can't open $custlist: $!";

# DERA,1000,1001,7000,9000,sdriuna@dera.bc.ca,604 682-0931ext 101604 317-5346,604 669-5499,Sabrina Driuna,12 E. Hastings St. Vancouver BC,Invoice,
my @cfields = qw/cust vid newbox rangestart rangeend email phone fax contact address paytype/;
my (%customers);
while (my $cline = <CUST>) {
	chomp $cline;
	
	my %cdata; @cdata{@cfields} = split ",", $cline;
	$cdata{vendor} = $cdata{cust};
	if ($cdata{paytype} =~ /:gstexempt/) {
		$cdata{gstexempt} = 1;
	} else {
		$cdata{gstexempt} = 0;
	}
	if ($cdata{cust} eq 'DERA') {
		$cdata{rate} = 0;
	} else {
		$cdata{rate} = 2.5;
	}
	$customers{$cdata{cust}} = \%cdata;
}
close CUST;
print Dumper(\%customers) if $opt{v} > 1;

=pod
| vid          | int(11)      | NO   | PRI | NULL              | auto_increment | 
| vendor       | varchar(128) | NO   |     |                   |                | 
| created      | timestamp    | NO   |     | CURRENT_TIMESTAMP |                | 
| parent       | int(11)      | YES  |     | NULL              |                | 
| address      | varchar(128) | YES  |     | NULL              |                | 
| phone        | varchar(128) | YES  |     | NULL              |                | 
| contact      | varchar(128) | YES  |     | NULL              |                | 
| email        | varchar(128) | YES  |     | NULL              |                | 
| fax          | varchar(128) | YES  |     | NULL              |                | 
| gstexempt    | int(11)      | YES  |     | 0                 |                | 
| rate         | float        | YES  |     | NULL              |                | 
| months       | int(11)      | YES  |     | 0                 |                | 
| all_months   | int(11)      | YES  |     | 0                 |                | 
| pst_number   | varchar(128) | YES  |     | NULL              |                | 
| gst_number   | varchar(128) | YES  |     | NULL              |                | 
| credit_limit | float        | YES  |     | NULL              |                | 
| status       | varchar(32)  | YES  |     |                   |                | 
| notes        | text         | YES  |     | NULL              |                | 
=cut

my @vfields = qw/vendor address phone contact email fax gstexempt rate months/;
my $updvendq = "update vendors set ".(join ",", map { "$_=?" } @vfields)." where vid=?";
print "$updvendq\n" if $opt{v};
my $updvend = $ldb->prepare($updvendq);

foreach my $cust (@active) {
	warn "no vendor id for $cust!" and next unless defined $customers{$cust};
	$customers{$cust}{months} = $counts{$cust}{ttl};
	print join ",", @{$customers{$cust}}{@vfields},"\n" if $opt{v};
	$updvend->execute(@{$customers{$cust}}{@vfields},$customers{$cust}{vid})
		or die $updvend->errstr;
}

=pod
| invoice | int(11)     | NO   | PRI | NULL                | auto_increment | 
| created | timestamp   | NO   |     | CURRENT_TIMESTAMP   |                | 
| paidon  | timestamp   | NO   |     | 0000-00-00 00:00:00 |                | 
| months  | int(11)     | YES  |     | NULL                |                | 
| gst     | float       | YES  |     | NULL                |                | 
| pst     | float       | YES  |     | NULL                |                | 
| total   | float       | YES  |     | NULL                |                | 
| notes   | mediumtext  | YES  |     | NULL                |                | 
| vid     | int(11)     | YES  |     | NULL                |                | 
| login   | varchar(32) | YES  |     | NULL                |                | 
5004,Invoice,ZTest,2002/05/26,10,1.64,25.00,2002/05/26
=cut
my @ifields = qw/invoice paytype vendor created months gst total paidon/;
my @insinvfields = qw/invoice created paidon months gst total vid/;
my $invoices = "$pcdir/invoices.csv";
open INV, "< $invoices" or die "can't open $invoices: $!";

my $insinvq = "replace into invoices (".(join ",", @insinvfields).") values (".(join ",",map {'?'} @insinvfields).")";
print "$insinvq\n" if $opt{v};
my $insinv = $ldb->prepare($insinvq);
while (my $iline = <INV>) {
	print $iline if $opt{v};
	chomp $iline;
	my %idata; @idata{@ifields} = split ",", $iline; 
	$idata{vid} = $customers{$idata{vendor}}{vid};
	$insinv->execute(@idata{@insinvfields}) or die $insinv->errstr;
}
close INV;

