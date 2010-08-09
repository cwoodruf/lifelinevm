#!/usr/bin/perl
use Lifeline::DB;
use LWP::Simple;
use Data::Dumper;
use Getopt::Std;
use strict;
my %opt;
getopts('v', \%opt);

my $auth_token = '8fa0b3a8d3029ed9fecf0e35071d7842';
my $client_account_id = '80015026';
my $baseurl = "http://developer.voicemeup.com/xml_engine/xml_engine.php";
my $auth = "client_account_id=$client_account_id&auth_token=$auth_token";

my $day = shift;
$day = `/bin/date +\%Y-\%m-01` unless $day =~ /^\d\d\d\d-\d\d-\d\d/;
my $url = "$baseurl?$auth&request_type=get_calls&date_from=$day";
print "$url\n" if $opt{v};
my $xml = get($url);

die "error getting $xml: $!" unless defined $xml;
print "$xml\n" if $opt{v};

my @calls;

=filter out
    <call_details unique_id="6a45b9a125d141151c8500e71a85a3af">
      <date>2010-08-04</date>
      <time>12:39:47</time>
      <direction>inbound</direction>
      <source>6047751806</source>
      <destination>16042484930</destination>
      <client_peer_id>5507</client_peer_id>
      <country_id>38</country_id>
      <country>Canada</country>
      <state_id>2</state_id>
      <state>British Columbia</state>
      <district>Richmond</district>
      <duration>82</duration>
      <callerid_name>6047751806</callerid_name>
      <callerid_number>6047751806</callerid_number>
      <bindings></bindings>
      <status>answered</status>
      <normalized>16042484930</normalized>
      <billed_amount>0.0241</billed_amount>
      <current_rate>0.0172</current_rate>
      <monthly_usage>increment</monthly_usage>
    </call_details>
=cut
my @fields = qw/
	date
	time
	direction
	source
	destination
	client_peer_id
	country_id
	country
	state_id
	state
	district
	duration
	callerid_name
	callerid_number
	bindings
	status
	normalized
	billed_amount
	current_rate
	monthly_usage
/;
my $insq = "replace into voicemeupcalls (unique_id,".(join ",", @fields).") ".
		"values (?,".(join ",", map {"?"} @fields).")";
my $ins = $ldb->prepare($insq);
while ($xml =~ m#<call_details unique_id="(.*?)">(.*?)</call_details>#sg) {
	my $id = $1;
	my $call = $2;
	my %rec;
	foreach my $field (@fields) {
		if ($call =~ m#<$field>(.*?)</$field>#s) {
			$rec{$field} = $1;
		}
	}
	$ins->execute($id,@rec{@fields}) or die "call:\n".Dumper(\%rec).$ins->errstr;
}


