#!/usr/bin/perl
# purpose of this script is to do a one time dump of customer information into the vendors db
use Lifeline::DB;
use strict;

my $pcdir = '/usr/local/asterisk/paycode';
my $custlist = "$pcdir/custlist.csv";
open CUST, "< $custlist" or die "can't open $custlist: $!";
=pod
DERA,1000,1001,7000,9000,sdriuna@dera.bc.ca,604 682-0931ext 101604 317-5346,604 669-5499,Sabrina Driuna,12 E. Hastings St. Vancouver BC,Invoice,QrZRlquyDxdP6
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

my @infields = qw/vendor vid new startbox endbox email phone fax contact address paymenttype pass/;
my @outfields = qw/vid vendor parent address phone contact email fax gstexempt rate status/;
my $insq = "insert ignore into vendors (".(join ",", @outfields).") values (".(join ",", map { "?" } @outfields).")";
my $ins = $ldb->prepare($insq);

while (my $line = <CUST>) {
	print $line;
	chomp $line;
	my %data; @data{@infields} = split ",", $line;
	$data{parent} = 1000;
	$data{gstexempt} = $data{paymenttype} =~ /:gstexempt/ ? 1 : 0;
	$data{status} = 'deleted'; # reactivate them based on how many months they've paid for
	$data{rate} = 2.5;
	$ins->execute(@data{@outfields}) or die $ins->errstr;
}
close CUST;

