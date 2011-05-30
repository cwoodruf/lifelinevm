#!/usr/bin/perl
# insert hand grabbed voicemeup logs see also ...-xmlapi.pl for the xml version
use Lifeline::DB;
use Getopt::Std;
use strict;
my %opt;
getopts('q',\%opt);

# scan all files given: the first line in each file should be the field names
my (@fields, $ins, $insq);
while (<>) {
	chomp;
	unless (m/"/) {
		@fields = split ",";
		$insq = "update voicemeupcalls set ".(join ",", map { "$_=?" } @fields).",ts=? ".
			"where unique_id=?";
		$ins = $ldb->prepare($insq);
		next;
	}
	warn "no fields!\n$_" and next unless defined @fields;
	print "adding $_\n" unless $opt{q};
	my %data; @data{@fields} = my @values = map { s/^"//; s/"$//; $_ } split '","';
	$ins->execute(@values,$data{date}.' '.$data{'time'},$data{call_hash}) or die $ins->errstr.":\n$insq\n@values";
}
