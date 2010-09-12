<?php
session_start();
define('MAXREQUESTS',15);
?>

<Html>
<head>
<link rel=stylesheet type=text/css href="css/index.css">
<title>Voice mail reminder opt out</title>
</head>
<body vlink=#660099 link=#686868 alink=#686868 bgcolor=lightyellow>
<center>
<h3>Lifeline Voice Mail Email Optout Form</h3>

<?php

if ($_REQUEST['email']) {
	$_SESSION['requests']++;
	if ($_SESSION['requests'] > MAXREQUESTS) {
		print "You have made too many requests. Please wait and try again later.\n";
	} else if ($_REQUEST['email'] != $_REQUEST['confirm_email']) {
		print "Error: emails do not match!\n";
	} else {
		$email = htmlentities(trim($_REQUEST['email']));
		if (preg_match('#^\w[\w\-\.]*@\w[\w\-\.]*\.\w{2,4}$#',$email)) {
			if (($fh = fopen("../bin/llremind/optout/".md5($email),'w')) !== false) {
				print "<b>$email</b> will not receive further voicemail reminders.\n";
				fwrite($fh, $email);
				fclose($fh);
			} else {
				print "Error saving $email!\n";
			}
		} else {
			print "Error: invalid email $email.\n";
		}
	}

} else {
	print <<<HTML
Fill in the following form to stop email reminders:

<form action=optout.php method=post>
<table cellpadding=5 cellspacing=0 border=0>
<tr><td>Email:</td><td><input name=email size=40></td></tr>
<tr><td>Confirm Email:</td><td><input name=confirm_email size=40></td></tr>
<tr><td colspan=2 align=right><input type=submit name=action value="Opt Out"></td></tr>
</table>
</form>

HTML;
}
?>

</center>
</body>
</html>

