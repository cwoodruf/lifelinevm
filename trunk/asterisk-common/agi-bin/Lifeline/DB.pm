package Lifeline::DB;
# convenience module for importing DERA vm data into the default db
# there may be other databases 
use Exporter;
use DBI;
our @ISA = qw/Exporter/;

our @EXPORT = qw/$lldatafile $ldb $llmsgdir $lleol ll_deleted_boxes/;
 
our $dsn = "DBI:mysql:database=lifeline;host=localhost";
our $username = 'll';
our $password = '';
our $ldb = DBI->connect($dsn,$username,$password) or die DBI::errstr;
END { $ldb->disconnect if defined $ldb; }

our $lldatafile = "/usr/local/asterisk/Lifeline/users.csv";
our $llmsgdir = "/usr/local/asterisk/lifeline-msgs";
# end of life in days
our $lleol = 90;

sub ll_deleted_boxes {
	my %delbox;
	my $getq = "select box from boxes where status = 'deleted'";
	my $get = $ldb->prepare($getq);
	$get->execute or die $get->errstr;
	while (my $row = $get->fetch) {
		$delbox{$row->[0]} = 1;
	}
	$get->finish;
	\%delbox;
}

1;
