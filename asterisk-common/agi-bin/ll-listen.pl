#!/usr/bin/perl
# purpose of this script is to play messages for a voicemail box
use Lifeline;
use strict;
=filter out

from lifeline's ansrvar.h

       {sNOMSGS, "No messages message.", nomsgs, nomsgs_cmplt, "NOMSGS.VOX", 0, 0},//13
       {sNEWMSGS, "New messages message.", nomsgs, nomsgs_cmplt, "NEWMSGS.VOX", 0, 0},//13
       {sLASTMSG, "Last message message.", lastmsg, lastmsg_cmplt, "LASTMSG.VOX", 0, 0},//14
       {sDELETEDMSG, "Message deleted message.", lastmsg, deletedmsg_cmplt, "DELMSG.VOX", 0, 0},//15
       {sWASDELETED, "Message had been deleted message.", lastmsg, wasdeleted_cmplt, "WASDEL.VOX", 0, 0},//15
       {sRESTOREDMSG, "Message restored message.", lastmsg, lastmsg_cmplt, "RESTMSG.VOX", 0, 0},//16
       {sDELETE, "Deleting messages.", delall, delall_cmplt, "DELETE.VOX", 0, 0},//17
       {sRESTORE, "Restoring messages.", delall, delall_cmplt, "RESTORE.VOX", 0, 0},//18
       {sDATEINTRO, "Message recorded on...", dateintro, dateintro_cmplt, "MSGREC.VOX", 0, 0},//19

=cut

my $prompt = 'll-en-listen';
my $no_msgs = 'll-en-nomsgs';
my $last_msg = 'll-en-lastmsg';
my $was_del = 'll-en-wasdel';
my $del_msg = 'll-en-delmsg';
my $restore_msg = 'll-en-restmsg';
my $del_all = 'll-en-delete';
my $restore_all = 'll-en-restore';
my $msg_rec = 'll-en-msgrec';
my $extplay = 'll-en-extplay';
my $digits = '1234567890*#';

# initialize lifeline
my $ll = Lifeline->init();
$ll->load_msgs();
$ll->flag_new_msgs(0);
$ll->play_msg_count or exit;
my $a = $ll->{agi};
my $callback_app;

# start with the first message
PLAY:
while ((my $ascii = &play($ll->curr_msg,$callback_app)) >= 0) {
	MENU:
	if ($ascii <= 0) {
		$ascii = $a->stream_file($prompt,$digits);
		exit if $ascii <= 0;
	}
	$callback_app = undef;
	my $d = chr($ascii);
	$ascii = 0;
	if    ($d eq '1') { $ll->inc_msg; } 
	elsif ($d eq '2') { $ll->dec_msg; }
	elsif ($d eq '3') { }
	elsif ($d eq '4') { $ll->first_msg; }
	elsif ($d eq '5') { $ll->last_msg; }
	elsif ($d eq '*') { last PLAY; }
	elsif ($d eq '#') { &msg_date($ll->curr_msg); goto MENU; }
	elsif ($d eq '7') { 
		$ll->del_msg; 
		if ($ll->curr_msg->{deleted}) {
			$a->stream_file($del_msg); 
		} else {
			$a->stream_file($restore_msg);
		}
		goto MENU; 
# removed this as its too confusing and probably not so many will use it
#	} elsif ($d eq '8') { 
#		$ascii = $a->stream_file($extplay,$digits);
#		$d = chr($ascii);
#		$ascii = 0;
#		if ($d eq '8') { $callback_app = ['ControlPlayback','2000|3|1|8|2']; }
#		else { goto MENU; }
	} elsif ($d eq '9') { 
		$ll->del_all($ll->curr_msg->{deleted}); 
		if ($ll->curr_msg->{deleted}) {
			$a->stream_file($del_all);
		} else {
			$a->stream_file($restore_all);
		}
		goto MENU; 
	} else { goto MENU; }
}
END { $ll->clean_up_msgs; }

sub play {
	my ($msg,$cb) = @_;
	exit unless defined $msg;
	my $k;
	$k = $a->stream_file($was_del,$digits) if $msg->{deleted};
	return $k if $k;
	my $mfile = "$msg->{msg}.$ll->{rectype}";
	my $deleted = "$msg->{msg}.deleted.$ll->{rectype}";
	# note, any message file could disappear mid-play for any reason
	# this is just an attempt at being polite in a multiuser environment
	if (-f $mfile) { 
		# everything is ok
		if (defined $cb) {
			$cb->[1] = "$msg->{msg}|$cb->[1]";
			$k = $a->exec(@$cb);
		} else {
			$k = $a->stream_file($msg->{msg},$digits);
		}
	} elsif (-f $deleted) { 
		# somebody else deleted this message while you were on the phone 
		$k = $a->stream_file($was_del,$digits) if $msg->{deleted};
		return $k if $k;
		$deleted =~ s/\.$ll->{rectype}$//;
		if (defined $cb) {
			$cb->[1] = "$deleted|$cb->[1]";
			$k = $a->exec(@$cb);
		} else {
			$k = $a->stream_file($deleted,$digits);
		}
	} else { 
		# should probably have a prompt here - the message is completely gone in this case
		return $k; 
	}
	return $k if $k;
	$k = $a->stream_file($last_msg,$digits) if $msg->{last};
	$k;
}

sub msg_date {
	my ($msg) = @_;
	exit unless defined $msg;
	$a->stream_file($msg_rec);
	my $msg_epoch = ($msg->{msg}=~m#^.*/(\d+)#)[0];
	$a->say_date($msg_epoch);
	$a->exec("Wait","1");
	$a->say_time($msg_epoch);
}

