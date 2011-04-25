#!/usr/bin/perl
use Lifeline::DB;
use LWP::Simple;
use Data::Dumper;
use Getopt::Std;
use strict;
my %opt;
getopts('vm:t:', \%opt);

my $auth_token = 'add token here';
my $client_account_id = '80015026';
my $baseurl = "http://developer.voicemeup.com/xml_engine/xml_engine.php";
my $auth = "result_limit=1000&client_account_id=$client_account_id&auth_token=$auth_token";

my ($day,$endday,$date_to,$url,$unique_id,$err);
$day = shift;
if ($day ne '') {
	$endday = shift;
	$date_to = "&date_to=$endday" if $endday =~ /^\d\d\d\d-\d\d-\d\d/;
	$url = "$baseurl?$auth&request_type=get_calls&date_from=$day$date_to";
} else {
	my $get_last_ts = $ldb->prepare("select max(ts) from voicemeupcalls");
	if ($get_last_ts->execute) {
		if ($get_last_ts->rows) {
			my $last_ts = ($get_last_ts->fetch)->[0];
			my $get_last_call = $ldb->prepare(
				"select * from voicemeupcalls where ts='$last_ts'"
			);
			if ($get_last_call->execute) {
				my $last_call = $get_last_call->fetchrow_hashref;
				if (defined $last_call) {
					$unique_id = $last_call->{unique_id};
					$url = "$baseurl?$auth&request_type=get_calls&unique_id=$unique_id";
				}
			} 
		}
	}
	unless (defined $unique_id) {
		warn "couldn't get last call - using date instead";
		$day = `/bin/date +\%Y-\%m-\%d`;
		$url = "$baseurl?$auth&request_type=get_calls&date_from=$day";
	}
}
print "$url\n" if $opt{v};
my $xml = get($url);

die "error getting $xml: $!" unless defined $xml;
# print "$xml\n" if $opt{v};

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
my ($count,$notanswered);
$count = $notanswered = 0;
while ($xml =~ m#<call_details unique_id="(.*?)">(.*?)</call_details>#sg) {
	my $id = $1;
	my $call = $2;
	my %rec;
	foreach my $field (@fields) {
		if ($call =~ m#<$field>(.*?)</$field>#s) {
			$rec{$field} = $1;
		}
	}
	$count++;
	$notanswered++ unless $rec{status} eq 'answered';
	my $insq = "insert ignore into voicemeupcalls (unique_id,".(join ",", @fields).",ts) ".
			"values (".
				$ldb->quote($id).",".
				(join ",", map {$ldb->quote($_)} @rec{@fields}).",".
				$ldb->quote("$rec{date} $rec{'time'}").
			")";
	print "$insq\n" if $opt{v};
	my $ins = $ldb->prepare($insq);
	$ins->execute or die $ins->errstr;
}
print scalar(localtime),": notanswered $notanswered, count $count, threshold $opt{t}, email $opt{m} ";
if ($opt{m} =~ /\@/ and defined $opt{t}) {
	if ($notanswered >= $opt{t}) {
		print "sending email";
		open MAIL, "| /usr/bin/mail -s 'unanswered calls' '$opt{m}'";
		print MAIL "unanswered $notanswered, total $count";
		close MAIL or die "error sending mail to $opt{m}: $!";
	}
}
print "\n";

