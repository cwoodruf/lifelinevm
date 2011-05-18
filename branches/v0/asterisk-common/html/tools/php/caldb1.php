<?php
	$cdb = pg_connect("dbname=caldb1 user=cal") or die(pg_last_error());
	# according to their documtentation the pg_close func is not needed normally
?>
