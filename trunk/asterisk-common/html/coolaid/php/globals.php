<?php

# coolaid specific
$sitename = 'Coolaid';
$sitecolor = '#F8F8F8';

# note that some of these global values are used by the 
# html/tools/php/mysql.php and html/tools/php/pw/auth.php shared libraries

# vendor id for setting rate etc for a vendor
define("ROOTVID",6962);
define("MAXBOXES",20); 
define("MAXMONTHS",24); 
define('GETASTSTATUS', false);
define("DELBOXAFTER",90);
define("DAY",86400);
define("MINBOX",1000);
define("MAXBOX",9999);
define("DEFRATE",2.5);
define("DEFPRICES","1=3.00;2=6.00;3=9.00;4=10.00;5=12.50;6=15.00;7=17.00;8=19.00;9=21.00;10=23.00;11=25.00;12=25.00");
define("INVOICEOVERDUE", 90);
define("INVOICEBLOCKED", INVOICEOVERDUE + 30);

# default phone number if the vendor doesn't have a phone associated with them
define("DEFPHONE","604 248-4930");
$phone = DEFPHONE;

# where things are
$lib = '/usr/local/asterisk/html/tools/php';
define("DBLOGINFILE","$lib/.mysql-coolaid.php");
define("SALTFILE","/usr/local/asterisk/agi-bin/Lifeline/salt-coolaid");
$asterisk = '/usr/local/asterisk/sbin/asterisk';
$asterisk_dir = "/usr/local/asterisk/var/lib/asterisk";
$asterisk_sounds = "$asterisk_dir/sounds";
$asterisk_lifeline = "/usr/local/asterisk/coolaid-msgs";
$asterisk_rectype = "gsm"; # normal file recording extension
$asterisk_deleted = "deleted"; # extension to use to "delete" files
$lifeline_root = "/usr/local/asterisk/html/coolaid";

# allowable range of boxes when we want to create a box
function get_box_range() { 
	return array(2000,9999);
	# these ranges avoid new boxes from the DERA vm service
	# we do this so both can be used at the same time without collisions
	$ranges = array(array(6001,6999),array(9001,9999));
	return $ranges[mt_rand(0,1) > 0.5 ? 1 : 0]; 
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
# credit for number of months: -1 = no credit limit
$def_credit_limit = -1; 
# how long to keep a box alive if its not renewed 
# see Lifeline.pm - this should be same value
$pt_cutoff = 3 * 7 * 84600; 


