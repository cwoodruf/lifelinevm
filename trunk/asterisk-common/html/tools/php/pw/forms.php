<?php

function login_form ($app) {
	global $lib;
	if (@include_once("$lib/asterisk.php")) {
		$uptime = get_uptime();
	}
	$vars = array_merge($_GET,$_POST);
	foreach ($vars as $key => $val) {
		if (preg_match('#^(login|password|cache|app)$#',$key)) continue;
		$hidden .= "<input type=hidden name=\"$key\" value=\"$val\">\n";
	}
	$login = htmlentities($vars['login']);
	return <<<HTML
<html>
<head><title>Login</title></head>
<body>
<h3>{$_SERVER['SERVER_NAME']} log in</h3>
<form name=form_login action=$app method=post>
$hidden
<table cellpadding=3 cellspacing=0 border=0>
<tr><td><b>Login:</b></td>
    <td><input name=login size=30 maxlength=30 value="$login">
    <script>document.form_login.login.focus()</script></td></tr>
<tr><td><b>Password:</b></td>
    <td><input type=password name=password size=30 maxlength=30></td></tr>
<tr><td><input type=reset value='Reset'></td>
    <td align=right><input type=submit name=action value="Log In"></td></tr>
</table>
</form>
<span style="font-size: small">
Voice Mail $uptime.
<script>
var gt = '>';
var lt = '<';
var at = '@';
var dot = '.';
document.write(lt);
document.write('a');
document.write(' ');
document.write('h');
document.write('r');
document.write('e');
document.write('f');
document.write('=');
document.write('"');
document.write('m');
document.write('a');
document.write('i');
document.write('l');
document.write('t');
document.write('o');
document.write(':');
document.write('v');
document.write('m');
document.write('a');
document.write('i');
document.write('l');
document.write('t');
document.write('e');
document.write('c');
document.write('h');
document.write('n');
document.write('i');
document.write('c');
document.write('a');
document.write('l');
document.write('s');
document.write('u');
document.write('p');
document.write('p');
document.write('o');
document.write('r');
document.write('t');
document.write(at);
document.write('g');
document.write('m');
document.write('a');
document.write('i');
document.write('l');
document.write(dot);
document.write('c');
document.write('o');
document.write('m');
document.write('"');
document.write(gt);
document.write('S');
document.write('u');
document.write('p');
document.write('p');
document.write('o');
document.write('r');
document.write('t');
document.write(lt);
document.write('/');
document.write('a');
document.write(gt);
</script>
</span>
</body>
</html>
HTML;
}

function print_login ($app) {
	echo login_form($app);
	exit;
}

?>
