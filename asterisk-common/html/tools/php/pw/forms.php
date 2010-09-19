<?php

function login_form ($redirecturl,$app,$callback) {
	global $lib,$newsdiv;
	if (@include_once("$lib/asterisk.php")) {
		$uptime = get_uptime();
	}
	$vars = $_REQUEST; # array_merge($_GET,$_POST);
	foreach ($vars as $key => $val) {
		if (preg_match('#^(login|password|cache|app|callback)$#',$key)) continue;
		$hidden .= "<input type=hidden name=\"$key\" value=\"$val\">\n";
	}
	$login = htmlentities($vars['login']);
	return <<<HTML
<html>
<head>
<title>Login</title>
<style type=text/css>
* {
	font-family: arial, verdana, sans-serif;
	color: brown;
}
a {
	color: green;
}
input {
	color: black;
	font-size: 12pt;
}
table {
	background: white;
	width: 500px;
	border: 1px black solid;
}
div.news {
	text-align: left;
	width: 500px;
	font-size: 10pt;
	border: 1px black solid;
	background: white;
	margin: 15px;
	padding: 10px;
}
li.news {
	padding-bottom: 10px;
}
li.news b {
	color: black;
}
</style>
</head>
<body bgcolor=lightyellow>
<center>
<h3>{$_SERVER['SERVER_NAME']} log in</h3>
<div>
<form name=form_login action="$redirecturl" method=post>
<input type=hidden name=app value="$app">
<input type=hidden name=callback value="$callback">
$hidden
<table cellpadding=3 cellspacing=0 border=0>
<tr><td><b>Login:</b></td>
    <td><input name=login size=64 maxlength=64 value="$login">
    <script>document.form_login.login.focus()</script></td></tr>
<tr><td><b>Password:</b></td>
    <td><input type=password name=password size=64 maxlength=64></td></tr>
<tr><td><input type=reset value='Reset'></td>
    <td align=right><input type=submit name=action value="Log In"></td></tr>
</table>
</form>
</div>
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
$newsdiv
</center>
</body>
</html>
HTML;
}

function print_login ($redirecturl,$app,$callback) {
	echo login_form($redirecturl,$app,$callback);
	exit;
}

?>
