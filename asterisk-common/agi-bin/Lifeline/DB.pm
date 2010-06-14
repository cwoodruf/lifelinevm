package Lifeline::DB;
# convenience module for importing DERA vm data into the default db
# there may be other databases 
use Exporter;
use DBI;
our @ISA = qw/Exporter/;

our @EXPORT = qw/$lldatafile $ldb/;
 
our $dsn = "DBI:mysql:database=lifeline;host=localhost";
our $username = 'll';
our $password = '';
our $ldb = DBI->connect($dsn,$username,$password) or die DBI::errstr;
END { $ldb->disconnect if defined $ldb; }

our $lldatafile = "/usr/local/asterisk/Lifeline/users.csv";

1;
