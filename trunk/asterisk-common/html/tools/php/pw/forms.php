<?php

function login_form ($app) {
	global $_GET;
	global $_POST;
	$vars = array_merge($_GET,$_POST);
	foreach ($vars as $key => $val) {
		if (preg_match('#^(login|password|cache|app)$#',$key)) continue;
		$hidden .= "<input type=hidden name=\"$key\" value=\"$val\">\n";
	}
	return <<<HTML
<html>
<head><title>Login</title></head>
<body>
<h3>Lifeline Log in</h3>
<form name=form_login action=$app method=post>
$hidden
<table cellpadding=3 cellspacing=0 border=0>
<tr><td><b>Login:</b></td>
    <td><input name=login size=30 maxlength=30>
    <script>document.form_login.login.focus()</script></td></tr>
<tr><td><b>Password:</b></td>
    <td><input type=password name=password size=30 maxlength=30></td></tr>
<tr><td><input type=reset value='Reset'></td>
    <td align=right><input type=submit name=action value="Log In"></td></tr>
</table>
</form>
</body>
</html>
HTML;
}

function print_login ($app) {
	echo login_form($app);
	exit;
}

?>
