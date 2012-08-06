<?php
/*
if ($fh = fopen('iplog.txt','a')) {
	fwrite($fh,date('Y-m-d H:i:s')." ".$_SERVER['REMOTE_ADDR']." ".$_SERVER['PHP_SELF']."\n");
	fclose($fh);
}
*/
if ($_SERVER['REMOTE_ADDR'] != '65.36.160.216') {
	header("content-type: text/plain");
	die("unrecognized");
}
