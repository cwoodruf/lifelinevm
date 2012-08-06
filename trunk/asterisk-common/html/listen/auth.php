<?php
#ini_set('display_errors',true);
#error_reporting(E_ALL & ~E_NOTICE);
require("checkip.php");
header('content-type: text/plain');

$key = $_GET['key'];
if (!preg_match('#^[a-f0-9]{40}$#',$key) or !is_link($key)) return;
require("../lifeline/php/globals.php");
require_once("../tools/php/mysql.php");
if (($valid = ll_valid_email_user($_GET))) {
	print "OK";
	return;
}
require("../coolaid/php/globals.php");
if (ll_valid_email_user($_GET)) {
	print "OK";
	return;
}
