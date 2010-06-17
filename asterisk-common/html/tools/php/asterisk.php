<?php
# some asterisk related routines 

function get_uptime() {
	global $asterisk, $lib;
	static $uptime;

	if (isset($uptime)) return $uptime;

	# only rebuild every once in a while 
	$cache = "$lib/.asterisk-uptime";

	if (!is_file($cache) or filesize($cache) < 20 or time() - filemtime($cache) > 600) {
		$uptime = `$asterisk -rx 'core show uptime'`;
		file_put_contents($cache,$uptime);
	}
	return ($uptime = file_get_contents($cache));
}

