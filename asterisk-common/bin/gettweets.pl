#!/usr/bin/perl
# purpose of this script is to read and cache new tweets from twitter for log in news
use LWP::Simple;
use PHP::Serialization qw(serialize unserialize);
use JSON;
use Getopt::Std;
use Data::Dumper;
use Date::Manip;
use strict;
my %opt;
getopts('l:c:f:vh',\%opt);
my $options = <<TXT;
gettweets.pl [-l{login}] [-c{count}] [-f{output file}] [-vh]
 -l is the twitter login
 -c is the number of tweets to download 
 -f is the output file for serialized php data
gettweets.pl gets and parses twitter content and saves it in a
way that is easy for a php page to use. 
gettweets.pl is intended to be run occasionally as a cron job.
TXT
print $options and exit if $opt{h};

my $deflogin = 'lifelinevm';
my $me = $opt{l} || $deflogin;
my $count = $opt{c} || 4;
my $verbose = $opt{v};

my $serialfile = "/usr/local/asterisk/html/lifeline/tweets.serialized";
if ($opt{f}) { 
	$serialfile = $opt{f};
}

my $apiurl = "http://api.twitter.com/statuses/user_timeline.json?screen_name=$me&count=$count";
print STDERR $apiurl,"\n" if $verbose;

my $json = get($apiurl);
die "error getting!" unless $json;

my $ary = decode_json($json);
die "error decoding!" if ref $ary ne 'ARRAY';
my %tweets;
foreach my $tweet (@$ary) {
	(my $tweeted = ParseDate($tweet->{created_at})) =~ s/(\d{4})(\d{2})(\d{2})(.*)/$1-$2-$3 $4/;
	$tweets{$tweeted} = $tweet->{text};
}

my $serialized = serialize(\%tweets);
open TWEETS, "> $serialfile" or die "can't open $serialfile: $!";
print TWEETS $serialized;
close TWEETS;
