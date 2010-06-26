<?php
# used in the ll_new_box function
function get_box_range() { 
	# these ranges avoid new boxes from the DERA vm service
	# we do this so both can be used at the same time without collisions
	$ranges = array(array(6001,6999),array(9001,9999));
	return $ranges[mt_rand(0,1) > 0.5 ? 1 : 0]; 
}

$adminfo = '<a class="support" href="mailto:vmailtechnicalsupport@gmail.com">Support</a>';
$asterisk = '/usr/local/asterisk/sbin/asterisk';
define('GETASTSTATUS', false);

# default permissions list: there should be no other user permssions
# these determine what you can create
$baseperms = array('boxes','invoices','logins','vendors');

$min_purchase = 4; # least number of months you can buy
$def_credit_limit = -1; # credit for number of months: -1 = no credit limit
$lib = '/usr/local/asterisk/html/tools/php';
$pt_cutoff = 3 * 7 * 84600; # see Lifeline.pm - this should be same value
$asterisk_dir = "/usr/local/asterisk/var/lib/asterisk";
$asterisk_sounds = "$asterisk_dir/sounds";
$asterisk_lifeline = "/usr/local/asterisk/lifeline-msgs";
$asterisk_rectype = "gsm"; # normal file recording extension
$asterisk_deleted = "deleted"; # extension to use to "delete" files
$lifeline_root = "/usr/local/asterisk/html/lifeline";
