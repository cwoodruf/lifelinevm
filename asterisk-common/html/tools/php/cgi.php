<?php
require_once("$lib/globals.php");
require_once("$lib/args.php");

function get_thumb_params() {
	list($img,$max_dim,$quality) = get_cgi_params(array('img','max_dim','quality'));
	$max_dim = round($max_dim);
	$quality = round($quality);
	if (preg_match('#[`;]#',$img)) die("invalid characters in image name $img!");
	if (!is_numeric($max_dim) or $max_dim < 10 or $max_dim > 500)
		die("invalid max_dim $max_dim should be: 10-500");
	if (!is_numeric($quality) or $quality < 0 or $quality > 100)
		die("invalid quality $quality should be: 0-100");
	return array($img,$max_dim,$quality);
}

