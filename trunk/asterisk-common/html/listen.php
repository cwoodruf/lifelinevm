<?php
date_default_timezone_set('America/Vancouver');
$header = file_get_contents('header.php');
$footer = file_get_contents('footer.php');
$howto = file_get_contents('listenhowto.php');
require_once('recaptchalib.php');
session_start();

$baseurl = "https://lifelinevm.net/listen";
$key = $_REQUEST['key'];

if ($_GET['action'] == 'logout') {
	$_SESSION['box'] = false;
	$key = $_SESSION['key'];
	unset($_SESSION['key']);
}
if (!$_SESSION['box']) {
	if (!login()) return;
}
$box = $_SESSION['box'];

if (preg_match('#^[a-f0-9]{40}$#',$key)) {
	$msg = $_GET['msg'];
	if (preg_match('#^\d+\.\d+\.gsm$#',$msg)) {
		if ($_GET['text'] == '1') {
			$details = file_get_contents("$baseurl/details.php?key=$key&msg=$msg");
			if (preg_match('#callerid = ([^;]*);#',$details,$cid)) {
				$callerid = $cid[1];
			} else {
				$callerid = "no Caller Id available";
			}
			print <<<HTML
<html>
<head>
<title>Caller id</title>
<link rel=stylesheet type=text/css href="css/index.css">
</head>
<body>
<table class="maintable"><tr><td>
<div align="center" class="panel" style="width: 255px; border-top: 1px black solid;">
<b>Caller Id for message $msg</b>
<br>
$callerid
</div></td></tr></table>
</body>
</html>
HTML;
		} else {
			header("content-type: audio/gsm");
			header("content-disposition: attachment; filename=$msg");
			readfile("$baseurl/send.php?key=$key&msg=$msg");
		}
		return;
	}
	$files = file("$baseurl/list.php?key=$key");
	foreach ($files as $file) {
		list($key,$msg) = split("/",trim($file));
		if (!preg_match('#^[a-f0-9]{40}$#',$key)) continue;
		if (!preg_match('#^\d+\.\d+\.gsm$#',$msg)) continue;

		$time = preg_replace('#^(\d+).*#',"$1",$msg);
		$when = date('Y-m-d H:i:s',$time);
		$msgs .= <<<HTML
<a href="listen.php?key=$key&msg=$msg">$when</a> - 
<a href="javascript: void(0);" 
   onclick="window.open('listen.php?key=$key&msg=$msg&text=1','callerid',
	'menubar=no,toolbar=no,statusbar=no,addressbar=no,width=300,height=100,screenX=100,screenY=100'); 
	return false;"
>caller id</a><p>
HTML;
	}
	if (!$msgs) { $msgs = "No new messages found. Try calling in directly."; }
	print <<<HTML
<html>
<head>
<title>Messages for box {$_SESSION['box']}</title>
$header
<a href="listen.php?action=logout">Log out</a> -
$howto  
<h3>Recent messages for box $box - click to listen</h3>
$msgs
$footer
HTML;
}

function login() {
	global $baseurl, $key, $header, $footer;

	$captcha = '';
	$captcha_ok = true;
	$captcha_ok = check_captcha();
	$box = trim($_POST['box']); 
	$seccode = trim($_POST['seccode']);
	$email = trim($_POST['email']);
	if (
		$captcha_ok and $box and $email and $seccode
	) {
		if (
			preg_match('#^[0-9a-f]{40}$#',$key) and 
			preg_match('#^\d{4}$#',$box) and 
			preg_match('#^\d{4}$#',$seccode) and 
			preg_match('#^[\w\.\-]+@[\w\-\.]+\.[a-z]{2,4}$#',$email)
		) {
			$encemail = urlencode($email);
			$loginurl = "$baseurl/auth.php?key=$key&email=$encemail&box=$box&seccode=$seccode";
			$response = file_get_contents($loginurl);
			if (preg_match('#OK\s*#',$response)) {
				$_SESSION['box'] = $box;
				$_SESSION['key'] = $key;
				$_SESSION['captcha'] = false;
				return true;
			}
		}
	} 
	$captcha = mk_captcha();
	print <<<HTML
<html>
<head>
<title>Log in</title>
$header
<h3>Log in</h3>
<form action=listen.php method=post>
<input type=hidden name=key value="$key">
<table cellpadding=5 cellspacing=0 border=0>
<tr>
<td>Email: </td><td><input name=email size=30 value="$email"></td>
</tr><tr>
<td>Voice mail box number: </td><td><input name=box value="$box"></td>
</tr><tr>
<td>Security Code: </td><td><input type=password name=seccode></td>
</tr><tr>
<td colspan=2 align=right>
$captcha
<input type=submit name=action value="Log In">
</td>
</table>
</form>
$footer
HTML;
	return false;
}

function mk_captcha() {
	$_SESSION['captcha'] = true;
	return recaptcha_get_html('6Le1EMsSAAAAAFPz7tddZbfrbeEP8kywc0-QiJf9');
}

function check_captcha() {
	$privatekey = "6Le1EMsSAAAAAFfP80-gplqRsedIFNpTOXwZ1yaa";
	$resp = recaptcha_check_answer ($privatekey, $_SERVER["REMOTE_ADDR"], 
			$_POST["recaptcha_challenge_field"], $_POST["recaptcha_response_field"]);
	return $resp->is_valid;
}

