<?php
# some asterisk related routines 

function get_uptime() {
	global $asterisk, $lib;
	static $uptime;

	if (isset($uptime)) return $uptime;

	$uptime = `$asterisk -rx 'core show uptime'`;

	return $uptime;
}

