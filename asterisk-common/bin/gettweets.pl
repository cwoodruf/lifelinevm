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
getopts('l:c:v',\%opt);

my $deflogin = 'lifelinevm';
my $me = $opt{l} || $deflogin;
my $count = $opt{c} || 3;
my $verbose = $opt{v};

my $serialfile = "/usr/local/asterisk/html/lifeline/tweets.serialized";
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
