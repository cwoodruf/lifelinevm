<?php
# user interface for listening to messages
require_once('php/globals.php');
require_once("$lib/pw/auth.php");
$from = $_REQUEST['from'];
if (!preg_match('#^\w*$#',$from)) unset($from);
if (preg_match('#^(?:admin|make)$#',$from)) {
	$ldata = login_response("/lifeline/$from.php",'ll_pw_data');
	$box = $_REQUEST['box'];
	ll_check_box($box);
	$bdata = ll_box($box);
	if ($bdata['vid'] != $ldata['vid']) {
		die("Error: vendor mismatch! You may not have permission to view this box.");
	}
} else {
	$ldata = login_response($_SERVER['PHP_SELF'],'ll_user_data');
	$box = $ldata['login'];
	ll_check_box($box);
	$bdata = ll_box($box);
}
$bdir = "$asterisk_lifeline/$box/messages";

$action = $_REQUEST['action'];
$filter = $_REQUEST['filter'];
if (!isset($filter) or !preg_match('#^(|current)$#',$filter)) $filter = 'current';

if      ($action === "delete") toggle_delete();
else if ($action === "play") play();
else if ($action === "logout") logout();
else if ($action === "All messages") $filter = '';
else if ($action === "Current messages only") $filter = 'current';

messages($filter);
ll_disconnect();

function logout() {
	delete_login();
	print "Thank you for visiting. <a href=\"/lifeline/listen.php\">log in again</a><br>\n";
	exit;
}
	
function messages($filter) {
	global $asterisk_rectype;
	global $asterisk_deleted;
	global $ldata;
	global $cache;
	global $bdata;
	global $bdir;
	global $box;
	global $from;
	if (preg_match('#^\w+$#',$from)) {
		$goback = "<a href=\"$from.php\">Go back to admin page</a><br>";
	}
	if ($filter === 'current') $table_title = "Current messages";
	else if ($filter === '') $table_title = "All messages";
	print <<<HTML
<html>
<head>
<title>Messages for box $box</title>
<link rel=stylesheet type=text/css href=/lifeline/css/listen.css>
</head>
<body bgcolor=lightyellow>
<center>
$goback
<h3>Messages for box $box (paid to: $bdata[paidto])</h3>
<form action=/lifeline/listen.php method=post>
<input type=hidden name=from value="$from">
<input type=hidden name=box value="$box">
<input type=submit name=action value="Current messages only"> &nbsp;
<input type=submit name=action value="All messages"> <br>
</form>
<b>$table_title (newest to oldest)</b>: <p>
<table cellpadding=8 cellspacing=0 border=0>
HTML;
	$items = glob("$bdir/*\.$asterisk_rectype");
	rsort($items);
	reset($items);
	foreach ($items as $item) {
		if ($filter === 'current') {
			if (!preg_match("#/((\d+)\.\d+\.$asterisk_rectype)$#",$item,$m)) continue;
		} else if (!preg_match("#/((\d+)\.\d+\.($asterisk_deleted\.|)$asterisk_rectype)$#",$item,$m)) continue;
		$date = date('Y-m-d g:i a',$m[2]);
		$stat = stat($item);
		$size = $stat['size'];
		if ($m[3] == '') { $del_action = 'delete'; $msg_state = ""; }
		else { $del_action = 'undelete'; $msg_state = "(deleted)"; }
		$count++;
		# removed this one for privacy reasons - can still delete / undelete messages
		$listenlink = <<<HTML
<td><a href="/lifeline/listen.php?action=play&from=$from&box=$box&filter=$filter&msg=$m[1]">listen</a> </td>
HTML;
		print <<<HTML
<tr valign=top>
<td><b>Message $count $msg_state :</b></td>
<td>$date </td>
<td>$size bytes</td>
<td><a href="/lifeline/listen.php?action=delete&from=$from&box=$box&filter=$filter&msg=$m[1]">$del_action</a> </td>
</tr>
HTML;
	}
	$no_msgs = $count ? "" : "No messages<br>\n";
	print <<<HTML
</table>
$no_msgs
</center>
</body>
</html>
HTML;
}

function toggle_delete() {
	global $asterisk_rectype;
	global $asterisk_deleted;
	global $bdata;
	global $bdir;
	global $box;
	$msg = $_REQUEST['msg'];
	if (preg_match("#^(\d+\.\d+)\.($asterisk_rectype)$#",$msg,$m)) {
# print "Deleting $msg<br>\n";
		$deleted = "$bdir/".$m[1].".$asterisk_deleted.".$m[2];
		$msg = "$bdir/$msg";
		if ($deleted !== $msg and is_file($msg)) {
			@unlink($deleted);
			rename($msg,$deleted);
		}
	} else if (preg_match("#^(\d+\.\d+)\.$asterisk_deleted\.($asterisk_rectype)$#",$msg,$m)) {
# print "Undeleting $msg<br>\n";
		$undeleted = "$bdir/".$m[1].".".$m[2];
		$msg = "$bdir/$msg";
		if ($undeleted !== $msg and is_file($msg)) {
			@unlink($undeleted);
			rename($msg,$undeleted);
		}
	}
}

function play() {
	global $asterisk_rectype;
	global $asterisk_deleted;
	global $bdata;
	global $bdir;
	global $box;
	$msg = $_REQUEST['msg'];
	if (preg_match("#^(\d+\.\d+)\.($asterisk_rectype)$#",$msg,$m)) {
		$path = "$bdir/$msg";
		header("Expires: Mon, 26 Jul 1997 05:00:00 GMT");
		header("Last-Modified: " . gmdate("D, d M Y H:i:s") . " GMT");
		header("Content-type: audio/gsm;"); 
		header("Content-Transfer-Encoding: binary");
		$len = filesize($path);
		header("Content-Length: $len;");
		header("Content-disposition: inline; filename=$msg");
		readfile($msg);
	}
}

