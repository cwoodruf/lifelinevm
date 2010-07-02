<?php
# purpose of this script is to remove the annoying dialog box that comes up from
# using the back button to go back to the main page after logging in by doing a
# redirect that removes all the login information but turns the request into a GET 
ob_start();
require_once('php/globals.php');
require_once("$lib/pw/auth.php");

$vars = array_merge($_GET,$_POST);

$ldata = login_response('redirect.php',$vars['app'],$vars['callback']);

$url = $vars['app'];
unset($vars['app']);

foreach ($vars as $name => $value) {
	if (preg_match('#^(login|password|PHPSESSID)$#',$name)) continue;
	$urlbits[] = urlencode($name).'='.urlencode($value);
}
if (count($urlbits)) {
	$url .= '?'.implode('&',$urlbits);
}
header("location: $url");
ob_end_flush();
