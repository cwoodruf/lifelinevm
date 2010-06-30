#!/usr/bin/perl
# purpose of this script is to update the lifeline db and "delete" old boxes
# in addition remove old messages that are either from deleted boxes or have "deleted" 
# attached to the message file name
# TODO: may also want to purge call logs at some later point
use Lifeline::DB;
use File::Find;
use File::Copy;
use Getopt::Std;
use strict;
my %opt;
getopts('v',\%opt);

# first set status on old boxes as deleted
my $delq = "update boxes set status='deleted' ".
	"where status <> 'deleted' and paidto > 0 and datediff(current_date(),paidto) > $lleol";
print "$delq\n" if $opt{v};
my $deleted = $ldb->do($delq) or die $ldb->errstr;
print "deleted $deleted boxes\n" if $opt{v};

# get the full list so we can mark all their messages as deleted
my $getdelq = "select box from boxes where status='deleted'";
my $getdel = $ldb->prepare($getdelq);
$getdel->execute or die $getdel->errstr;
my %oldbox;
while (my $row = $getdel->fetch) {
	$oldbox{$row->[0]} = 1;
}
$getdel->finish;

# now scan directories for stuff we should delete
finddepth(\&markdelete, $llmsgdir);
finddepth(\&purge, $llmsgdir);

sub markdelete {
	return unless $File::Find::name =~ m#(\d+)/messages/(\d+\.\d+)\.gsm#;
	my ($box,$stub) = ($1,$2);
	return unless $oldbox{$box};
	print "old? $oldbox{$box} - checking $box $stub $File::Find::name\n" if $opt{v};
	move $_, "$stub.deleted.gsm" or die "$_ -> can't mark as deleted: $!";
}

sub purge {
	return unless /(\d+)\.\d+\.deleted\.gsm/;
	my ($created) = ($1);
	my $deleteme = (time - (stat)[9] > $lleol * 86400);
	return unless $deleteme;
	print "purge check $File::Find::name delete? $deleteme\n" if $opt{v};
}

