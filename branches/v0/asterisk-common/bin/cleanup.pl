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
my %handles = (
	coolaid => Lifeline::DB::mkdbh('coolaid'),
	lifeline => $ldb,
);

# first set status on old boxes as deleted
our $oldbox;
foreach my $db (qw/lifeline coolaid/) {
	my $delq = "update boxes set status='deleted' ".
		"where status <> 'deleted' and status not like 'add % months' ".
		"and paidto > 0 and datediff(current_date(),paidto) > $lleol";
	print "$delq\n" if $opt{v};

	my $deleted = $handles{$db}->do($delq) or die $handles{$db}->errstr;
	print "deleted $deleted boxes\n" if $opt{v};

	my ($reverted,$revertedvends) = ll_revert_unused($lleol,$handles{$db});
	print "reverted $reverted allocated boxes unused for $lleol days for $revertedvends vendors\n" if $opt{v};

	# now scan directories for stuff we should delete
	foreach my $msgdir ("/usr/local/asterisk/$db-msgs") {
		$oldbox = ll_deleted_boxes($handles{$db});
		print scalar(keys %{$oldbox})," deleted boxes in $db.boxes\n" if $opt{v};
		finddepth(\&markdelete, $msgdir);
	}

}

sub markdelete {
	# look for old messages 
	if ($File::Find::name =~ m#(\d+)/(?:(greeting)\.gsm|messages/(\d+\.\d+)\.gsm)#) {
		my ($box,$grtstub,$msgstub) = ($1,$2,$3);
		my $stub = $grtstub || $msgstub;
		if ($oldbox->{$box}) {
			print "old? $oldbox->{$box} - checking $box $stub $File::Find::name\n" if $opt{v};
			move $_, "$stub.deleted.gsm" or die "$_ -> can't mark as deleted: $!";
		}
	# remove old messages that have been marked for deletion	
	} elsif (/\.deleted\.gsm$/) {
		my $deleteme = (time - (stat)[9] > $lleol * 86400);
		return unless $deleteme;
		print "purge check $File::Find::name delete? $deleteme\n" if $opt{v};
		unlink $_;
	}
}

