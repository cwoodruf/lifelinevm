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
foreach my $table (qw/lifeline.boxes coolaid.boxes/) {
	my $delq = "update $table set status='deleted' ".
		"where status <> 'deleted' and paidto > 0 and datediff(current_date(),paidto) > $lleol";
	print "$delq\n" if $opt{v};
	my $deleted = $ldb->do($delq) or die $ldb->errstr;
	print "deleted $deleted boxes\n" if $opt{v};

}

my ($reverted,$revertedvends) = ll_revert_unused($lleol);
print "reverted $reverted allocated boxes unused for $lleol days for $revertedvends vendors\n" if $opt{v};

# now scan directories for stuff we should delete
my $oldbox;
foreach my $msgdir (@llmsgdirs) {
	(my $table = $msgdir) =~ s#.*/(\w+)-msgs.*#$1.boxes#;
	$oldbox = ll_deleted_boxes($table);
	print scalar(keys %{$oldbox})," deleted boxes in $table\n" if $opt{v};
	finddepth(\&markdelete, $msgdir);
}

sub markdelete {
	# look for old messages and messages that have already been deleted (problem with rsyncing)
	if ($File::Find::name =~ m#(\d+)/(?:(greeting)\.gsm|messages/(\d+\.\d+)\.gsm)#) {
		my ($box,$grtstub,$msgstub) = ($1,$2,$3);
		my $stub = $grtstub || $msgstub;
		if ($oldbox->{$box}) {
			print "old? $oldbox->{$box} - checking $box $stub $File::Find::name\n" if $opt{v};
			move $_, "$stub.deleted.gsm" or die "$_ -> can't mark as deleted: $!";
		} elsif (-f "$stub.deleted.gsm") {
			print "markdelete removing $stub because its already deleted!\n" if $opt{v};
			unlink $_;
		}
	# remove old messages that have been marked for deletion	
	} elsif (/\.deleted\.gsm$/) {
		my $deleteme = (time - (stat)[9] > $lleol * 86400);
		return unless $deleteme;
		print "purge check $File::Find::name delete? $deleteme\n" if $opt{v};
		unlink $_;
	}
}

