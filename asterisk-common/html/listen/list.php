<?php
require("checkip.php");
header("content-type: text/plain");

$key = $_GET['key'];
if (preg_match('#^[a-f0-9]{40}$#',$key) and is_link($key)) {
	$msgs = glob("$key/[0-9]*[0-9].wav");
	rsort($msgs);
	print implode("\n", $msgs);
}
