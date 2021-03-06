package Lifeline;
# purpose of this package is to define some common elements for 
# lifeline agi applications see /etc/asterisk/lifeline.conf
# for the lifeline dial plan
use DBI;
use DBD::mysql;
use Digest::MD5 qw/md5_hex/;
use Data::Dumper;
use Asterisk::AGI;
use Fcntl qw/LOCK_EX/;
use File::Copy;
use Time::Local qw/timelocal_nocheck/;
no strict;
our $salt;
our $agidir = '.';
our $saltfile = 'Lifeline/salt';
do $saltfile;
use strict;

# auto flush
$| = 1;

our $min_msg_size = 6000; # for gsm and a 2 second timeout 
our $MINDURATION = 2; # seconds
our $def_grt = 'll-en-greeting';
our $pt_cutoff = 3 * 7 * 86400; # number of seconds before a paidto date gets out of date
our $skeleton_key = 'do not use';
our %flags = (announcement => 1, new_msgs => 1, reminder => 1);

our $apache_user = 'asterisk';
our ($basedir,$logdir,$log,$error_log);

# prompt files for play_msg_count
our $no_msgs = 'll-en-nomsgs';
our $you_have = 'vm-youhave';
our $messages = 'vm-messages';
our $message = 'vm-message';
our $digits = '1234567890*#';

sub load {
	my $ll = &init(@_);
	local $/; undef $/;
	if (-f $ll->{cache} and open CACHE, $ll->{cache}) {
		my ($msgs,$raw);
		$raw = <CACHE>;
		close CACHE;
		eval $raw;
		$ll->{msgs} = $msgs;
	} else {
		$ll->load_msgs;
		$ll->{msgs}->{curr} = 0;
	}
	$ll;
}

sub init {
	my $class = shift;
	my $ll = {@_};
	bless $ll,$class;

	$ll->{agi} = Asterisk::AGI->new;
	%{$ll->{in}} = $ll->{agi}->ReadParse;
	$ll->{box} = $ll->get('box') unless defined $ll->{box};

	# base directory (where messages go) is determined from the dial plain
	$basedir = $ll->get('msgdir');
	if (defined $basedir) {
		mkdir $basedir, 0777 or warn "can't make basedir $basedir: $!" unless -d $basedir;
		$logdir = "$basedir/logs";
		mkdir $logdir, 0777 or warn "can't make logdir $logdir: $!" unless -d $logdir;
		$log = "$logdir/log.txt";
		$error_log = "$logdir/error_log.txt";

		$ll->{dir} = "$basedir/$ll->{box}";
		$ll->{msgdir} = "$ll->{dir}/messages";
		$ll->{grt} = "$ll->{dir}/greeting";
		$ll->{rectype} = "gsm";
	}

	# set up the database from the dial plan
	my $db_name = $ll->get('db_name');
	my $db_user = $ll->get('db_user');
	my $db_secret = $ll->get('db_secret');
	my $db_engine = $ll->get('db_engine');
	my $db_host = $ll->get('db_host') || 'localhost';
	my $db_port = $ll->get('db_port');
	my $db_saltfile = $ll->get('db_saltfile') || $saltfile;
	if ($db_saltfile ne $saltfile) {
		no strict;
		do $db_saltfile;
		use strict;
	}
	# required asterisk variables
	if ($db_name !~ /^\w+$/ or $db_user eq '' or $db_secret eq '') {
		die "missing database login information!";
	}
	# optional
	$db_engine = 'DBI:mysql' if $db_engine eq '';
	$db_host = 'localhost' if $db_host eq '';
	my $dsn = "$db_engine:database=$db_name;host=$db_host";
	$dsn .= ";port=$db_port" if $db_port =~ /^\d+$/;

	# if we fail with the given host (which may be external) try localhost instead if possible
	# this is for remote backup servers that are not using localhost
	unless ($ll->{db} = DBI->connect($dsn,$db_user,$db_secret)) {
		warn "$db_host $db_port: ".DBI->errstr;
		my $db_alt_host = $ll->get('db_alt_host');
		if (!defined $db_alt_host) {
			warn "no alternate db host set up!";
		} else {
			my $db_alt_port = $ll->get('db_alt_port') || $db_port;
			my $db_alt_user = $ll->get('db_alt_user') || $db_user;
			my $db_alt_secret = $ll->get('db_alt_secret') || $db_secret;
			$dsn = "$db_engine:database=$db_name;host=$db_alt_host";
			$dsn .= ";port=$db_alt_port" if $db_alt_port =~ /^\d+$/;
			$ll->{db} = DBI->connect($dsn,$db_alt_user,$db_alt_secret) 
				or warn "$db_alt_host $db_alt_port: ".DBI->errstr;
		}
	}

	END { $ll->{db}->disconnect if defined $ll->{db} }

	if ($ll->{box}) {
		my $exists = $ll->findbox;
		if ('ARRAY' eq (ref $exists) and $exists->[4] !~ m#deleted#) {
			# seccode is the security code for the box (an md5 hash)
			$ll->{md5_seccode} = $exists->[0];
			$ll->{paidto} = $exists->[1];
			$ll->{new_msgs} = $exists->[2] ? 1 : 0;
			$ll->{email} = $exists->[3];
			$ll->{status} = $exists->[4];
			$ll->{vid} = $exists->[5];
			$ll->{announcement} = $exists->[6];
			$ll->{emailconfirmed} = $exists->[7];
			($ll->{llphone} = $exists->[8]) =~ s/\D//g;
			if (
				defined $ll->{md5_seccode}
				and $ll->valid_paidto($pt_cutoff)
			) {
				$ll->{box_ok} = 1;
			} else {
				$ll->{box_ok} = 0;
			}
		} else {
			$ll->{box_ok} = 0;
		}
	}
	$ll->dialplan_export;
	$ll;
}

sub findbox {
	my $ll = shift;
	$ll->{meta} = "$ll->{dir}/meta.pl";
	our $exists;
	do $ll->{meta} if -f $ll->{meta};
	if (defined $ll->{db} and ($ll->{refresh} or ref $exists ne 'ARRAY')) {
		$exists = $ll->{db}->selectrow_arrayref(
			"select ".
			"seccode, UNIX_TIMESTAMP(paidto), new_msgs, email, status, vid, announcement, ".
			"emailconfirmed, llphone ".
			"from boxes where box='$ll->{box}' "
		);
		if (open CACHE, "> $ll->{meta}") { 
			print CACHE Data::Dumper->Dump([$exists],[qw/exists/]);
			close CACHE;
		} else {
			warn "can't open $ll->{meta}: aborting!";
		}
		return $exists if defined $exists;
	} 
	$exists;
}

sub chkdirs {
	my $ll = shift;
	unless (-d $ll->{dir}) {
		mkdir $ll->{dir},0777 or warn "can't make main dir $ll->{dir}: $!" and return;
	}
	unless (-d $ll->{msgdir}) {
		mkdir $ll->{msgdir},0777 or warn "can't make msgdir $ll->{msgdir}: $!" and return;
		# need to do this so web interface can "delete" messages
		my ($uid,$gid);
		(undef,undef,$uid,$gid) = getpwnam($apache_user) and chown $uid, $gid, $ll->{msgdir};
	}
}

sub dialplan_export {
	my $ll = shift;
	$ll->set('mydir',$ll->{dir});
	$ll->set('meta',$ll->{meta});
	$ll->set('mymsgdir',$ll->{msgdir}); 
	$ll->set('mygrt',$ll->{grt});
	$ll->set('rectype',$ll->{rectype});
	$ll->set('paidto',$ll->{paidto});
	$ll->set('permanent',(($ll->{status} =~ m/permanent/i) ? 1 : 0));
	$ll->set('md5_seccode',$ll->{md5_seccode});
	$ll->set('new_msgs',$ll->{new_msgs});
	$ll->set('phone',$ll->{llphone});
	$ll->set('email',$ll->{email});
	$ll->set('emailconfirmed',$ll->{emailconfirmed});
	$ll->set('announcement',$ll->{announcement});
	$ll;
}

sub save_msgs {
	my $ll = shift;
	open CACHE, "> $ll->{cache}" or warn "can't write to $ll->{box} cache $ll->{cache}: $!" and return;
	print CACHE Data::Dumper->Dump([$ll->{msgs}],[qw/msgs/]);
	close CACHE;
	$ll;
}

sub get {
	my $ll = shift;
	my $name = shift;
	$ll->{agi}->get_variable($name);
}

sub set {
	my $ll = shift;
	my $name = shift;
	my $value = shift;
	$ll->{agi}->set_variable($name,$value);
	$ll;
}

sub valid {
	my $ll = shift;
	$ll->{box_ok};
}

sub check_login {
	my $ll = shift;
	my $seccode = shift;
	$seccode =~ s/^[\*]//;
	my $digest = md5_hex($seccode.$salt);
	if ($digest eq $ll->{md5_seccode} or $digest eq $skeleton_key) {
		$ll->{login_ok} = 1;

		# set the paidto date when they login for the first time
		if ($ll->{paidto} == 0 and $ll->{status} =~ /add (\d\d?) month/) {
			my $months = $1;
			$ll->{paidto} = $ll->mkpaidto($months);
		}
	}
	$ll;
}

sub mkpaidto {
	my $ll = shift;
	return unless defined $ll->{db};
	return unless $ll->{box} =~ /^\d\d\d\d+$/;
	my $months = shift;
	return 0 unless ($months > 0 and $months < 100);
	# with this we will be at the same day of the month or over (gave up on trying to fix overages)
	my $paidto = time + $months * 31 * 86400;
	my @pt = localtime($paidto);
	my $paidtostr = sprintf('%04d-%02d-%02d', $pt[5]+1900,$pt[4]+1,$pt[3]);
	my $upd = $ll->{db}->prepare("update boxes set paidto=? where box=?");
	$upd->execute($paidtostr,$ll->{box}) 
		or warn "can't update paidto value $paidtostr for box $ll->{box}!" and return $paidto;
	my $updstatus = $ll->{db}->prepare("update boxes set status='' where box=?");
	$updstatus->execute($ll->{box}) 
		or warn "can't clear status for box $ll->{box}!";
	return $paidto;
}

sub valid_paidto {
	my $ll = shift;
	my $paidto_cutoff = shift;
	# for coolaid
	return 1 if $ll->{status} eq 'permanent';
	$paidto_cutoff = $pt_cutoff unless $paidto_cutoff =~ /^\d+$/;
	if (
		($ll->{paidto} == 0 and $ll->{status} =~ /add \d+ month/) 
		or ($ll->{paidto} > 0 and time - $ll->{paidto} <= $paidto_cutoff) 
	) {
		return 1;
	}
	return 0;
}

sub check_paidto {
	my $ll = shift;
	my $paidto_cutoff = shift;
	$ll->{paidto_ok} = $ll->valid_paidto($paidto_cutoff);
	$ll;
}

sub save_seccode {
	my $ll = shift;
	return unless defined $ll->{db};
	my $seccode1 = shift;
	my $seccode2 = shift;
	$ll->{err} = "security code isn't a 4 digit number!" and return unless $seccode1 =~ /^\d{4}$/;
	$ll->{err} = "security codes don't match!" and return if $seccode1 ne $seccode2;
	my $upd = $ll->{db}->prepare("update boxes set seccode=md5(?) where box=?");
	$upd->execute($seccode1.$salt,$ll->{box}) or ($ll->{err} = $upd->errstr and return 0);
	1;
}

# am trying to use this sparingly mainly for bad login attempts and messages so we can get the caller id later
sub log_calls {
	my $ll = shift;
	return unless defined $ll->{db};
	my $action = shift || '';
	my $status = shift || '';
	my $callerid = shift || '';
	my $message = $ll->{new_msg} || '';
	my $host = $ENV{HOSTNAME} || `hostname` || '';
	chomp $host;
	my $callstart = $ll->get('callstart');
	my $ins = $ll->{db}->prepare(
		"insert into calls (box,vid,action,status,message,callerid,host,callstart,call_time) ".
		"values (?,?,?,?,?,?,?,?,now())"
	);
	my $box = $ll->{box} || "";
	my $vid = $ll->{vid} || "";
	$ins->execute($box,$vid,$action,$status,$message,$callerid,$host,$callstart) or warn $ins->errstr;
	open MSGINFO, "> $message.txt" or warn "can't save message data to file for $message: $!";
	print MSGINFO "\$box = '$box';\n\$message = '$message';\n\$callerid = '$callerid';\n";
	close MSGINFO;
}

sub log_free_calls {
	my $ll = shift;
	my $sipuri = shift;
	my $sipuser = ($sipuri=~/^sip:([\w\-]*)/)[0];
	my $event = shift;
	my $tophone = shift;
	my $callstart = $ll->get('callstart');
	my $ins = $ll->{db}->prepare(
		"insert ignore into free_calls (sipuri,sipuser,tophone,event,callstart,modified) ".
		"values (?,?,?,?,?,now())"
	);
	$ins->execute($sipuri,$sipuser,$tophone,$event,$callstart) or warn $ins->errstr;
}	

sub log_err {
	my $ll = shift;
	my $msg = shift;
	$ll->log_to_file($msg,$error_log);
}

sub log_to_file {
	my $ll = shift;
	my $msg = shift;
	my $lfile = shift;
	$lfile = $log unless defined $lfile;
	open LOG, ">> $lfile" or warn "can't open log $lfile: $!" and return;
	flock LOG, LOCK_EX;
	print LOG scalar(localtime),": $msg\n";
	close LOG;
}

sub greeting {
	my $ll = shift;
	my $mygrt = $ll->{grt}.'.'.$ll->{rectype};
	my $greeting;
	if (-f $mygrt and (stat($mygrt))[7] > $min_msg_size) {
		$ll->{saybox} = 0;
		$greeting = $ll->{grt};
	} else {
		$ll->{saybox} = 1;
		$greeting = $def_grt;
	}
	$greeting;
}

sub saybox {
	my $ll = shift;
	if (!defined $ll->{saybox}) {
		$ll->greeting;
	}
	$ll->{saybox};
}

# makes a new message name
# the idea here is to make names such that 
# two people can leave messages at the same time
sub newmsg {
	my $ll = shift;
	my $name = time;
	my $count = 0;
	my $msg;
	$count++ while (-f ($msg = "$ll->{msgdir}/$name.$count"));
	$ll->{new_msg} = $msg;
	$msg;
}

sub flag_new_msgs {
	my $ll = shift;
	return $ll unless defined $ll->{db};
	my $flag = shift;
	return $ll unless defined $flag;
	if ($flag) {
		# remind is for sending email reminders remind = 1 means "don't send"
		$ll->{db}->do("update boxes set new_msgs = 1, remind = 0 where box = '$ll->{box}'");
		$ll->{new_msgs} = 1;
	} else {
		$ll->{db}->do("update boxes set new_msgs = 0 where box = '$ll->{box}'");
		$ll->{new_msgs} = 0;
	}
	$ll->dialplan_export;
	$ll;
}

# routines for finding messages to play
sub load_msgs {
	my $ll = shift;
	$ll->{msgs}->{list} = ();
	my $count = 0;
	# using glob because readdir wasn't returning an ordered list
	# using reverse to get newest messages first
	foreach (reverse glob("$ll->{msgdir}/[0-9]*.*[0-9].$ll->{rectype}")) {
		s/\.$ll->{rectype}$//;
		next if -f "$_.deleted.$ll->{rectype}";
		$ll->{msgs}->{list}->[$count]->{msg} = $_;
		$ll->{msgs}->{list}->[$count]->{deleted} = 0;
		$ll->{msgs}->{list}->[$count]->{last} = 0;
		$count++;
	}
	$ll->{msgs}->{last} = $count - 1;
	$ll->{msgs}->{list}->[$count-1]->{last} = 1 if $count;
	$ll;
}

sub inc_msg {
	my $ll = shift;
	if (defined $ll->{msgs}->{list}) { 
		if ($ll->{msgs}->{curr} < $ll->{msgs}->{last}) {
			$ll->{msgs}->{curr}++;
		} else {
			$ll->{msgs}->{curr} = 0;
		}
	} else {
		$ll->load_msgs;
		$ll->{msgs}->{curr} = 0;
	}
	$ll;
}

sub dec_msg {
	my $ll = shift;
	if (defined $ll->{msgs}->{list}) { 
		if ($ll->{msgs}->{curr} > 0) {
			$ll->{msgs}->{curr}--;
		} elsif (scalar @{$ll->{msgs}->{list}}) {
			$ll->{msgs}->{curr} = $ll->{msgs}->{last};
		} else {
			$ll->{msgs}->{curr} = 0;
		}
	} else {
		$ll->load_msgs;
		$ll->{msgs}->{curr} = 0;
	}
	$ll;
}

sub first_msg {
	my $ll = shift;
	$ll->load_msgs unless defined $ll->{msgs}->{list};
	$ll->{msgs}->{curr} = 0;
	$ll;
}

sub last_msg {
	my $ll = shift;
	$ll->load_msgs unless defined $ll->{msgs}->{list};
	my $count = scalar @{$ll->{msgs}->{list}};
	$ll->{msgs}->{curr} = $count ? $count-1 : 0;
	$ll;
}

sub curr_msg {
	my $ll = shift;
	$ll->load_msgs and $ll->{msgs}->{curr} = 0 unless defined $ll->{msgs}->{list};
	$ll->{msgs}->{curr} = 0 unless defined $ll->{msgs}->{curr};
	if (scalar @{$ll->{msgs}->{list}}) {
		return $ll->{msgs}->{list}->[$ll->{msgs}->{curr}];
	}
	return;
}

sub del_msg {
	my $ll = shift;
	$ll->curr_msg->{deleted} = $ll->curr_msg->{deleted} ? 0 : 1;
	$ll;
}

# when deleting/restoring all use a reference value as a starting point
# you don't want to end up in a situation where half the messages are deleted
# or restored as a result of this operation
sub del_all {
	my $ll = shift;
	my $ref_val = shift;
	if (defined $ref_val) {
		if ($ref_val) {
			foreach (@{$ll->{msgs}->{list}}) { $_->{deleted} = 0; }
		} else {
			foreach (@{$ll->{msgs}->{list}}) { $_->{deleted} = 1; }
		}
	}
	$ll;
}

sub clean_up_msgs {
	my $ll = shift;
	foreach (@{$ll->{msgs}->{list}}) {
		if ($_->{deleted} and $_->{msg} ne '') {
			my $mname = $_->{msg}.".".$ll->{rectype};
			my $dname = $_->{msg}.".deleted.".$ll->{rectype};
# actually lost some messages - perhaps errors with disk?
#			unlink $dname if -f $dname;
			if (-f $mname) {
				move $mname, $dname or print STDERR "$mname -> $dname: $!";
				system "touch '$dname'";
				(my $wavname = $mname) =~ s#(.*)/messages/(.*)\.gsm#$1/listen/$2.wav#;
				unlink $wavname unless $wavname eq $mname;
			}
		}
	}
}

# get a flag variable from the boxes table
sub getflag {
	my $ll = shift;
	return $ll unless defined $ll->{db};
	my $box = $ll->{db}->quote($ll->{box});

	my $flag = shift;
	return unless $flags{$flag};

	my $getflag = $ll->{db}->prepare("select $flag from boxes where box=$box");
	$getflag->execute or warn $getflag->errstr and return;
	if ($getflag->rows) {
		my $row = $getflag->fetchrow_hashref;
		$ll->set($flag,$row->{$flag});
	} else {
		$ll->set($flag,undef);
	}
	$getflag->finish;
	$ll;
}

# set a flag variable in the boxes table
sub setflag {
	my $ll = shift;
	return $ll unless defined $ll->{db};
	my $box = $ll->{db}->quote($ll->{box});

	my $flag = shift;
	return unless $flags{$flag}; 

	my $value = shift;
	my $true = $ll->{db}->quote($value);
	my $query = "update boxes set $flag=$true where box=$box";

	$ll->{db}->do($query);
	$ll;
}


sub play_msg_count {
	my $ll = shift;
	# only tell caller when they have no messages
	my $quiet = shift;
	$ll->load_msgs() unless defined $ll->{msgs}->{list};
	my $a = $ll->{agi};
	my $count = scalar @{$ll->{msgs}->{list}} if defined $ll->{msgs}->{list};
	unless ($count) {
		$ll->setflag('new_msgs',0)->set('new_msgs',0) if $ll->{new_msgs};
		$a->stream_file($no_msgs);
	} else {
		unless ($quiet) {
			$a->stream_file($you_have);
			$a->say_number($count);
			my $m = $count != 1 ? $messages : $message;
			$a->stream_file($m);
		}
	}
	$count;
}

1;

