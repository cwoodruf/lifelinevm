<?php

function get_cgi_params($names) {
	if (!is_array($names)) return NULL;
	$vals = array();
	foreach ($names as $name) {
		array_push($vals,get_cgi_param($name));
	}
	return $vals;
}

function get_cgi_param($name) {
	$val = $_GET[$name] or $val = $_POST[$name];
	return $val;
}

function get_args (&$_args) {
	global $argv;
	if (!is_array($argv)) return 0;
	foreach ($argv as $arg) {
		$pair = explode('=',$arg);
		if (count($pair) > 1) {
			$name = array_shift($pair);
			$_args[$name] = join('',$pair);
		} else {
			$_args[$pair[0]] = NULL;
		}
	}
	return count($_args);
}
# directory where we are processing images
function get_thumbs_args ($get,$where = '',$min = 10,$max = 600) {
        $dir = $get['dir'];
        if (!preg_match('#^[\w/ ]+$#',$dir)) die("invalid characters in directory!");
        if ($where != '') {
                if (!preg_match("#$where#",$dir)) die("not in media tree!");
        }
        if (!is_dir($dir)) die("$dir is not a directory!");

	# overwrite thumbs?
	$overwrite = $get['overwrite'];
	if ($overwrite == '1') $overwrite = 1; else $overwrite = 0;

        # dimension to use to resize image
        $max_dim = $get['max_dim'];
        if (!preg_match("#^\d{2,3}$#",$max_dim)) die("bad maximum dimension!");
        if ($max_dim < $min or $max_dim > $max)
                die("invalid maximum dimension size (should be $min to $max)");

        # image quality desired
        $quality = $get['quality'];
        if (!preg_match("#^\d{1,3}$#",$quality)) die("bad quality number!");
        if ($quality < 0 or $quality > 100) die("bad quality value!");
        return array($dir,$overwrite,$max_dim,$quality);
}

