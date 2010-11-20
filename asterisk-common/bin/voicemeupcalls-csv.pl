#!/usr/bin/perl
# insert hand grabbed voicemeup logs see also ...-xmlapi.pl for the xml version
use Lifeline::DB;
use strict;

# scan all files given: the first line in each file should be the field names
my (@fields, $ins, $insq);
while (<>) {
	chomp;
	unless (m/"/) {
		@fields = split ",";
		$insq = "replace into voicemeupcalls (unique_id,".(join ",", @fields).") ".
				"values (?,".(join ",", map {"?"} @fields).")";
		$ins = $ldb->prepare($insq);
		next;
	}
	warn "no fields!\n$_" and next unless defined @fields;
print "adding $_\n";
	my %data; @data{@fields} = my @values = map { s/^"//; s/"$//; $_ } split ",";
	$ins->execute($data{call_hash},@values) or die $ins->errstr.":\n$insq\n@values";
}
