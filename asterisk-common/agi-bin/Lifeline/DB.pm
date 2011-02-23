package Lifeline::DB;
# convenience module for importing DERA vm data into the default db
# there may be other databases 
use Exporter;
use DBI;
our @ISA = qw/Exporter/;

our @EXPORT = qw/$coolaiddatafile $lldatafile @llmsgdirs $llmsgdir $coolaidmsgdir $ldb $lleol ll_deleted_boxes ll_revert_unused/;

our $username;
our $password;
our $port;
our $host;
# with coolaid I am cheating a bit as I've given the lifeline ll user permissions to work with coolaid's db
do "Lifeline/database";

our $dsn = &mkdsn($host,$port);
our $ldb = &mkldb($dsn,$username,$password);

END { $ldb->disconnect if defined $ldb; }

our @foreign = (
);

sub mkdsn {
	my ($host,$extra,$dbname) = @_;
	my $db = $dbname || 'lifeline';
	my $dsn = "DBI:mysql:database=$db;host=$host";
	if ($extra =~ /^\d+$/) {
		$dsn .= ";port=$extra";
	} elsif (-S $extra) {
		$dsn .= ";socket=$extra";
	} 
	$dsn;
}

sub drdatadb {
	no strict;
	do '/home/asterisk/agi/Lifeline/drdatadb';
	my $dsn = &mkdsn($drdatahost,$drdataport);
	DBI->connect($dsn,$drdatauser,$drdatapass) or die DBI::errstr;
	use strict;
}

sub mkdbh {
	my ($dbname) = @_;
	my $dsn = &mkdsn($host,$port,$dbname);
	my $dbh = DBI->connect($dsn,$username,$password) or die DBI::errstr;
	$dbh;
}

sub mkldb {
	my ($dsn,$username,$password) = @_;
	my $dbh = DBI->connect($dsn,$username,$password) or die DBI::errstr;
	$dbh;
}

our $lldatafile = "/usr/local/asterisk/Lifeline/users.csv";
our $coolaiddatafile = "/usr/local/asterisk/coolaid-msgs/lifeline/USERS.CSV";
our $llmsgdir = "/usr/local/asterisk/lifeline-msgs";
our $coolaidmsgdir = "/usr/local/asterisk/coolaid-msgs";
our @llmsgdirs = ($llmsgdir, $coolaidmsgdir);
# end of life in days
our $lleol = 90;

# just get a list of deleted boxes
sub ll_deleted_boxes {
	my ($dh,$table) = @_;
	$table = 'boxes' unless defined $table;
	$dh = $ldb unless defined $dh;
	my %delbox;
	my $getq = "select box from $table where status = 'deleted'";
	my $get = $dh->prepare($getq);
	$get->execute or die $get->errstr;
	while (my $row = $get->fetch) {
		$delbox{$row->[0]} = 1;
	}
	$get->finish;
	\%delbox;
}

# find all boxes older than $eoldays days that haven't been used
# as this func does everything for this task no need to die on error
# note this business logic does not apply to Coolaid where they
# pay a flat amount per month so its not implemented here
sub ll_revert_unused {
	my ($eoldays,$dh) = @_;
	$dh = $ldb unless defined $dh;
	warn "ll_revert_unused: invalid eoldays!" and return 
		unless $eoldays =~ /^\d+$/; # and $eoldays > 0;

	my $vids = $dh->selectall_arrayref("select vid,vendor from vendors") 
		or warn "ll_revert_unused: can't get vendor info! ".$dh->errstr and return;
	if (ref $vids ne 'ARRAY') {
		warn "ll_revert_unused: no vendor info!";
		return;
	}
	my %vendors = map { $_->[0],$_->[1] } @$vids;

	my $getq = "select box,vid,created,datediff(now(),created) as age,status from boxes ".
		"where (paidto = 0 or paidto is null) ".
		"and status regexp 'add [0-9][0-9]* months' ".
		"and datediff(now(),created) > '$eoldays'";

	my $get = $dh->prepare($getq);
	$get->execute or warn $get->errstr and return;

	my (%delete,%credit);
	while (my $row = $get->fetch) {
		my ($box,$vid,$created,$age,$status) = @$row;
		unless ($status =~ /add (\d+) months/) { 
			warn "ll_revert_unused: invalid status for $box $status";
			$get->finish; 
			return;
		}
		my $months = $1;
		warn "ll_revert_unused: don't know vendor $vid!" unless exists($vendors{$vid});
		
		$delete{$box} = $months;
		$credit{$vid} += $months;
	}
	$get->finish;

	foreach my $vid (keys %credit) {
		$dh->do("update vendors set months = months + '$credit{$vid}' where vid='$vid'")
			or warn $dh->errstr and return;
	}
	foreach my $box (keys %delete) {
		$dh->do("update boxes set notes = concat(notes,' ',status), status='deleted' where box='$box'")
			or warn $dh->errstr and return;
	}
	return (scalar (keys %delete), scalar (keys %credit));
}

1;
