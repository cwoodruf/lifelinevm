<?php

# coolaid specific
$sitename = 'Coolaid';
$sitecolor = '#F8F8F8';
define('DEFBOXLIMIT',200);
define('MININVOICE',8000);
define('MAXINVOICE',10000);

# note that some of these global values are used by the 
# html/tools/php/mysql.php and html/tools/php/pw/auth.php shared libraries

# vendor id for setting rate etc for a vendor
define("ROOTVID",1);
define("MAXBOXES",20); 
define("MAXMONTHS",24); 
define('GETASTSTATUS', false);
define("DELBOXAFTER",90);
define("DAY",86400);
define("MINBOX",1000);
define("MAXBOX",9999);
define("DEFRATE",2.5);
define("DEFPRICES","");
define("INVOICEOVERDUE", 90);
define("INVOICEBLOCKED", INVOICEOVERDUE + 30);
define("LOGINOUTPUTLIMIT",100);

# default phone number if the vendor doesn't have a phone associated with them
define("DEFPHONE","250 383-5144"); # this is just a victoria test number from link2voip.com
$phone = DEFPHONE;

# where things are
$lib = '/usr/local/asterisk/html/tools/php';
define("SALTFILE","/usr/local/asterisk/agi-bin/Lifeline/salt-coolaid");
$asterisk = '/usr/local/asterisk/sbin/asterisk';
$asterisk_dir = "/usr/local/asterisk/var/lib/asterisk";
$asterisk_sounds = "$asterisk_dir/sounds";
$asterisk_lifeline = "/usr/local/asterisk/coolaid-msgs";
$asterisk_rectype = "gsm"; # normal file recording extension
$asterisk_deleted = "deleted"; # extension to use to "delete" files
$lifeline_root = "/usr/local/asterisk/html/coolaid";

# allowable range of boxes when we want to create a box
if (!function_exists('get_box_range')) {
function get_box_range() { 
	return array(1000,9999);
	# these ranges avoid new boxes from the DERA vm service
	# we do this so both can be used at the same time without collisions
	$ranges = array(array(6001,6999),array(9001,9999));
	return $ranges[mt_rand(0,1) > 0.5 ? 1 : 0]; 
}
}

# link to show for support email
$adminfo = '<a class="support" href="mailto:vmailtechnicalsupport@gmail.com">Support</a>';

# default permissions list: there should be no other user permssions
# these determine what you can create
$baseperms = array('edit','boxes','invoices','logins','vendors');
$permhelp = array(
	'edit' => 'view  and edit box info only',
	'boxes' => 'create, delete or add time to a box',
#	'invoices' => 'buy voicemail', # removed for coolaid
	'logins' => 'create new logins for this vendor',
#	'vendors' => 'make new sub accounts', # removed for coolaid
);

# financial stuff
# least number of months you can buy
$min_purchase = 4; 
# see DEFBOXLIMIT above: coolaid has a cap on the number of boxes they can have 
# and they pay month to month more or less like the phone bill - except that I generate 
# the invoices and send them by email
# credit for number of months: -1 = no credit limit
$def_credit_limit = -1; 
# how long to keep a box alive if its not renewed 
# see Lifeline.pm - this should be same value
$pt_cutoff = 3 * 7 * 84600; 

# colors
$lightyellow = '#FFFFE0';
$cornsilk = '#FFF8DC';
$white = '#FFFFFF';
$lavender = '#E6E6FA';
$gray = '#909090';
$verylightgray = '#F8F8F8';
$lightgray = '#E3E3E3';
# database
# database
if (file_exists("/usr/local/asterisk/agi-bin/Lifeline/database")) {
        eval(file_get_contents("/usr/local/asterisk/agi-bin/Lifeline/database"));
} else if (file_exists("/usr/local/asterisk/var/lib/asterisk/agi-bin/Lifeline/database")) {
        eval(file_get_contents("/usr/local/asterisk/var/lib/asterisk/agi-bin/Lifeline/database"));
}
$ll_dbname = 'coolaid';
# personal information associated with a box - this should override mysql.php
$personal_fields = array(
        'name' => 'Name',
        'email' => 'Email',
        'paidto' => 'Paid to',
        'status' => 'Status',
        'notes' => 'Notes',
        'llphone' => 'Phone',
);

