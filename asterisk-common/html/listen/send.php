<?php
require("checkip.php");

$key = $_GET['key'];
if (preg_match('#^[a-f0-9]{40}$#',$key) and is_link($key)) {
	$msg = $_GET['msg'];
	if (preg_match('#^\d+\.\d+\.wav$#',$msg) and is_file("$key/$msg")) {
		header("content-type: audio/wav");
		# header("content-disposition: attachment; filename=$msg");
		header("content-length: ".filesize("$key/$msg"));
		@readfile("$key/$msg");
		return;
	}
}
header("content-type: text/plain");
print "could not find file!\n";
