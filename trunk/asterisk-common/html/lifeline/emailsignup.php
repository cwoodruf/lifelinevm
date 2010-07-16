<?php
# script to handle email signups: these must exist in the db 
require_once("php/globals.php");
require_once("$lib/pw/auth.php");
require_once("php/forms.php");
$emailpat = '#([a-z0-9_][\w\-\.]*@(?:(?:[a-z0-9\-]+)\.)+[a-z]{2,4})#';

$id = $_REQUEST['id'];
$action = $_REQUEST['action'];
if ($action === 'logout') delete_login();
$login = trim($_REQUEST['login']);
if (!preg_match('#^[\w\.\@ -]{0,64}$#',$login)) die("bad login value $login for adding or updating user!");

if (preg_match('#^\w{32}$#', $id)) {

	$vend = ll_vendor_from_id($id);
	if (!isset($vend['vid'])) die("invalid id code!");

	# this is someone logging in for the first time so make / update user in users table
	if ($action == 'Save Password') {
		$page = emailsignup($id,$vend);
	} else {
		$page = emailsignup_form($id,$vend);
	}
} else {

	$ldata = login_response('redirect.php',$_SERVER['PHP_SELF'],'ll_superuser');
	if (!empty($ldata['vid'])) die("no direct access {$ldata['vid']}!");
	$vend = ll_vendor(0);
	$vend['vendor'] = 'admin';

	if ($action == 'Create Invites') {
		$page = confirmsendinvites_form();
	} else {
		$page = sendinvites_form();
	}
}	

print form_top($vend,false,false,'post');
print $page;
print form_end($vend);

########################################## functions ##########################
function emailsignup($id,$user) {
	$password = $_REQUEST['password'];
	if ($user['email'] != $_REQUEST['email']) die("emails don't match!");
	$email = htmlentities($_REQUEST['email']);
	if ($password != $_REQUEST['password_confirm']) die("passwords don't match!");
	if (ll_add_user($user,$email,$password,$user['perms'])) {
		return "Saved new password for $email. <a href=\"admin.php?login=$email&action=logout\">Log in</a>";
	}
	return "Error saving password for $email!";
}

function emailsignup_form($id,$user) {
	$email = htmlentities($user['email']);
	$html = <<<HTML
<input type=hidden name=id value="$id">
<input type=hidden name=email value="$email">
<table cellpadding=5 cellspacing=0>
<tr><td colspan=2>
Enter a new password for $email:
</td>
</tr>
<tr>
<td>
Password: 
</td><td>
<input name=password value="" size=30 type=password>
</td>
</tr>
<tr>
<td>
Confirm Password: 
</td><td>
<input name=password_confirm value="" size=30 type=password>
</td>
</tr>
<tr><td colspan=2 align=right><input type=submit name=action value="Save Password"></td></tr>
</table>

HTML;
	return $html;
}

function confirmsendinvites_form() {
	global $emailpat;

	$defvend = false;
	if (preg_match('#^\d+$#',($vid = $_REQUEST['vid']))) {
		if (!ll_vendor($vid)) die("vendor $vid does not exist!");
		$defvend = ll_vendor($vid);
	} else unset($vid);
	$vendor = $defvend;

	$emaillist = explode("\n", $_REQUEST['emails']);

	foreach ($emaillist as $email) {
		$email = trim(strtolower(str_replace(",","",$email)));
		if (empty($email)) continue;
		if (!preg_match($emailpat, $email, $m)) {
			$warnings .= "email $email is not valid!<br>\n";
			continue;
		}
		$email = $m[1];
		if (!$defvend) $vendor = ll_vendor_from_email($email);
		if (!$vendor) {
			$warnings .= "can't find vendor for $email!<br>";
			continue;
		}
		$ids[$email] = ll_emailsignup_id($email,$vendor,$_REQUEST['perms']);
	}
	if (isset($warnings)) die($warnings);
	if (!is_array($ids)) {
		if (preg_match($emailpat, $defvend['email'], $m)) {
			$email = $m[1];
			$ids[$email] = ll_emailsignup_id($email,$defvend,$_REQUEST['perms']);
		}
	}

	foreach (array('emails','perms','subject','emailtext') as $name) {
		$value = htmlentities($_REQUEST[$name]);
		$hidden .= "<input type=hidden name=\"$name\" value=\"$value\">\n";
	}
	$html = <<<HTML
<h4>Click on the email links to send invitation.</h4>
<input type=submit name=action value="Back to Invites Form">
$hidden

HTML;
	$subject = htmlentities($_REQUEST['subject']);
	foreach ($ids as $email => $id) {
		$email = htmlentities($email);
		$emailtext = htmlentities(preg_replace('#id=ID#',"id=$id",$_REQUEST['emailtext']));
		$body = str_replace('+','%20',urlencode(str_replace("\r","",$emailtext)));
		$link = "<a href=\"mailto:$email?subject=$subject&body=$body\" target=_blank>$email</a>";
		$emailtext = preg_replace('#(https?://\S+)#',"<a href=\"$1\" target=_blank>$1</a>",$emailtext);
		$html .= <<<HTML
<p>
<b>send to $link</b>
<p>
<table cellpadding=0 cellspacing=5>
<tr><td>
<pre>$emailtext</pre>
</td></tr>
</table>

HTML;
	}
	return $html;
}

function sendinvites_form() {
	$perms = htmlentities($_REQUEST['perms']);
	if (empty($perms)) $perms = 'edit:boxes:invoices:logins';

	$emails = htmlentities($_REQUEST['emails']);
	$vendors = ll_vendors(0);
	$vendsel = "<select name=vid><option>\n";
	foreach ($vendors as $vendor) {
		$vendsel .= "<option value=\"{$vendor['vid']}\">{$vendor['vendor']}</option>\n";
	}
	$vendsel .= "</select>\n";

	if (empty($_REQUEST['subject'])) {
		$subject = "Voice mail web admin login";
	} else $subject = htmlentities($_REQUEST['subject']);

	if (empty($_REQUEST['emailtext'])) {
		$emailtext = <<<TXT
DO NOT REPLY DIRECTLY TO THIS MESSAGE.

This is an automatic message from the Lifeline voice mail system.

Please click on the link below to create a new password to the voice mail web admin:

https://lifelinevm.net/lifeline/emailsignup.php?id=ID

If you have any questions contact us at:
vmailtechnicalsupport@gmail.com

Thank you!
TXT;
	} else $emailtext = htmlentities($_REQUEST['emailtext']);

	$html = <<<HTML
<input type=checkbox name="existingcontact" checked> 
Use <a href="make.php?action=emails" target=_blank>existing contact info</a> to get vendor id automatically 
<p>
<b>OR</b> 
<p>
Select vendor $vendsel
<p>
Permissions <input size=20 value="$perms" name="perms">
<p>
<b>Emails:</b><br>
<textarea name=emails rows=5 cols=40>$emails</textarea>
<p>
<b>Email text:</b><br>
Subject: <input name=subject size=70 value="$subject">
<br>
Message body:<br>
<textarea name=emailtext rows=10 cols=80 wrap=hard>$emailtext</textarea>
<p>
<input type=reset value="Reset Form"> &nbsp;&nbsp;
<input type=submit name=action value="Create Invites">

HTML;
	return $html;
}

