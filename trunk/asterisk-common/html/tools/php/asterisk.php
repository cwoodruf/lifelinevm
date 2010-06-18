<?php
# some asterisk related routines 

function get_uptime() {
	global $asterisk, $lib;
	static $uptime;

	if (isset($uptime)) return $uptime;
	$cache = "$lib/.asterisk-uptime";
	
	if (!is_file($cache) or time() - filemtime($cache) > 60) {
		$uptime = `$asterisk -rx 'core show uptime'`;
		file_put_contents($cache,$uptime);
	} else {
		$uptime = file_get_contents($cache);
	}
	if (empty($uptime)) $uptime = "VOICEMAIL SYSTEM DOWN!";

	return $uptime;
}

