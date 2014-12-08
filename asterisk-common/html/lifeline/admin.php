<?php
require_once("php/globals.php");
require_once("$lib/pw/auth.php");
require_once("twitterapi.php");
@include_once("news.php");
if (function_exists('gettweets')) $newsdiv = gettweets();

$action = $_REQUEST['action'];
if ($action === 'logout') delete_login();
/* 
# twitter stuff - experimental
$twitter_actions = array(
	'tweet' => true,
	'followers' => true,
	'deltweet' => true,
	'timeline' => true,
);

# for the export action
if ($_REQUEST['hash']) {
	$_REQUEST['password'] = base64_decode($_REQUEST['hash']);
}
$ldata = login_response('redirect.php',$_SERVER['PHP_SELF'],'ll_pw_data');
# only do this check for me as jobwave surrey is getting errors

if ($ldata['app'] != $_SERVER['PHP_SELF']) die("you are already logged in to {$ldata['app']}!");
$vdata = ll_vendor($ldata['vid']);

# temporarily become another vendor
$switchto = $_REQUEST['switch_vendor'];
# check to see if the request is sane at all
if (preg_match('#^\d+$#',$switchto) and $switchto != $ldata['vid']) {
	# check to see if we are allowed to be this vendor
	if ($switchto == $ldata['initial_vid'] or ll_has_access($ldata,ll_vendor($switchto))) {
		$ldata['orig_vid'] = $_SESSION['login']['orig_vid'] = $ldata['vid'];
		$ldata['orig_vendor'] = $_SESSION['login']['orig_vendor'] = $vdata['vendor'];
		$ldata['vid'] = $_SESSION['login']['vid'] = $switchto;
	}
}

# check for overdue invoices early 
# this check absolutely prevent someone from buying voice mail if they have sub vendors
if ($ldata['vid'] > 0) {
	$overdue = ll_invoices_overdue($ldata['vid'],$overdueblock);
}

$vdata = ll_vendor($ldata['vid']);
if ($vdata['status'] == 'deleted') {
	die("Error: {$vdata['vendor']} was deleted!");
}

# last minute sanity check
if (!ll_has_access($ldata['vid'],$vdata)) {
	header('location: /lifeline/admin.php');
	exit;
}

# something wonky happened as we can't find this vendor
if ($vdata == null or !is_array($vdata)) {
	delete_login();
	die("missing vendor: aborting!");
}

$myperms = split(':',$ldata['perms']);
foreach ($myperms as $p) {
	$permcheck[$p] = true;
}

if ($action === 'export' and $permcheck['export']) {
	$export = ll_vendor($_REQUEST['vid']);
	if (preg_match('#^\d+$#', $_REQUEST['vid']) and ll_has_access($ldata,$export)) {
		header('content-type: text/plain');
		$logins = ll_users($export);
		foreach ($logins as $login) {
			$exported[$login['login']] = $login['password'];
		}
		print base64_encode(serialize($exported));
	}
	exit;
}

$ldata = array_merge($ldata,$vdata);

# if we need to do something with a specific subvendor then
# we should grab that information here
if (isset($_REQUEST['vid'])) {
	$vdata = ll_vendor($_REQUEST['vid'],$ldata['vid']);
	if (!ll_has_access($ldata['vid'],$vdata)) {
		die("Error: you do not have access to subvendor {$vdata['vendor']}!");
	}
	# if the vendor was deleted offer to undelete them
	if ($vdata['status'] == 'deleted') {
		if ($action === 'undelete_vendor') {
			ll_undelete_vendor($vdata);
		} else {
			print <<<HTML
<h3>Vendor {$vdata['vendor']} was deleted!</h3>
<a href="admin.php?action=undelete_vendor&vid={$vdata['vid']}">undelete {$vdata['vendor']}</a>
HTML;
			exit;
		}
	}
}

if (!$permcheck['edit']) die("you do not have sufficient access to use this site!");

$form = $_REQUEST['form'] ? $_REQUEST['form'] : $_REQUEST['action'];
# for people who can only view / edit box info
if ($ldata['perms'] == 'edit') {
	unset($_REQUEST['listen']);
	$allowed_forms = array(
		'chsc' => true,
		'edit' => true,
		'View your voicemail boxes' => true,
		'View payments' => true,
		'View call events' => true,
		'Update name, email etc.' => true,
		'find_boxes' => true,
		'Search Boxes' => true,
		'showcode' => true,
		'Call Activity' => true,
	);
	if (!$allowed_forms[$form]) {
		$action = '';
		$_REQUEST['search'] = 'add [0-9]* month';
		$form = 'Search Boxes';
	}
}

if (preg_match('#^\d+$#', $_REQUEST['vid'])) {
	$findvid = $_REQUEST['vid'];
	$finddata = ll_vendor($findvid);
	if (!ll_has_access($ldata,$finddata)) die("you are not associated with this vendor!");
} else {
	$findvid = $ldata['vid'];
	$finddata = $ldata;
}
*/
# this should be equivalent to the commented out section above
require_once("php/ldata.php");
require_once("php/forms.php");

/*
# twitter stuff - experimental
if ($twitter_actions[$action]) {
	ini_set('display_errors',true);
	error_reporting(E_ALL & ~E_NOTICE);
	header("content-type: text/plain");
	$ta = new TwitterAPI;
}
*/
if ($_REQUEST['listen']) {
	header("location: listen.php?from=admin&box={$_REQUEST['box']}");
} else if ($action === 'Manage account and users') {
	header("location: make.php?from=admin");
} else if ($action === 'Create boxes') {
	print create_new_box($ldata);
} else if ($action === 'Create box') {
	print create_new_box($ldata,$_REQUEST['box']);
} else if ($action === 'Really add time to box?' or $action === 'Really remove time from box?') {
	print update_box_time($ldata);
} else if ($action === 'Delete box') {
	print confirm_delete_form($ldata);
} else if ($action === 'Really delete box') {
	print delete_box($ldata);
} else if ($action === 'Change security code') {
	print confirm_update_seccode($ldata);
} else if ($action === 'Really change security code') {
	print update_seccode($ldata);
} else if ($action === 'Update name, email etc.') {
	print update_personal($ldata);
} else if ($action === 'Recent logins') {
	print list_logins($ldata,$_REQUEST['limit']);
} else if ($action === 'Show boxes') {
	print view_boxes($ldata);
} else if ($action === 'Purchase time') {
	print confirm_purchase_form($ldata);
} else if ($action === 'Buy voicemail now') {
	print purchase_time($ldata);
} else if ($action === 'invoice') {
	print invoice($ldata);
/*
# twitter stuff - experimental
} else if ($action === 'followers') {
	var_export($ta->ids());
} else if ($action === 'tweet') {
	var_export($ta->tweet($_REQUEST['tweet']));
} else if ($action === 'deltweet') {
	var_export($ta->destroy($_REQUEST['tweetid']));
} else if ($action === 'timeline') {
	var_export(json_decode($ta->timeline()));
*/
} else {
	if ($form === 'Create a new voicemail box') {
		print create_new_box_form($ldata);
	} else if ($form === 'Get free phone logs') {
		$fflogs = ll_free_phone_log($ldata['vid'],$_REQUEST['from'],$_REQUEST['to']);
		print show_free_phone_logs($fflogs);
	} else if ($form === 'Call Activity') {
		print box_activity($ldata);
	} else if ($form === 'showcode') {
		print showcode($ldata, sprintf('%04d',$_REQUEST['box']), $_REQUEST['seccode'],'html');
	} else if ($form === 'View transaction' or $form === 'transaction') {
		print view_transaction($ldata,ll_valid_trans($_REQUEST['trans']));
	} else if ($form === 'Add time to box' or $form === 'Remove time from box') {
		print confirm_update_box_time($ldata);
	} else if ($form === 'Add time to an existing box' or $form === 'add') {
		print update_box_form($ldata);
	} else if ($form === 'sub') {
		print update_box_form($ldata,'Remove time from box');
	} else if ($form === 'del') {
		# print update_box_form($ldata,'Delete box');
		$form = "Confirm voice mail box delete";
		print confirm_delete_form($ldata);	
	} else if ($form === 'chsc') {
		print update_box_form($ldata,'Change security code');
	} else if ($form === 'edit') {
		print update_box_form($ldata,'Update name, email etc.');
	} else if ($form === 'View your voicemail boxes' or $form === 'find_boxes') {
		print find_boxes_form($finddata);
	} else if ($form === 'View call events') {
		print find_calls_form($finddata);
	} else if ($form === 'View payments') {
		print find_payments_form($finddata);
	} else if ($form === 'Search Boxes') {
		print find_boxes_form($finddata,ll_find_boxes($findvid,$_REQUEST['search']));
	} else if ($form === 'Find Box') {
		print find_boxes_form($finddata,ll_find_box($findvid,$_REQUEST['box'],$_REQUEST['seccode']));
	} else if ($form === 'Show all invoices') {
		print list_invoices($vdata,true);
	} else if ($form === 'Show unpaid invoices') {
		print list_invoices($vdata,false);
	} else if ($form === 'Edit invoice') {
		print edit_invoice($_REQUEST['invoice']);
	} else if ($form === 'Save invoice') {
		if (ll_check_invoice($ldata,$_REQUEST) and ll_save_invoice($_REQUEST,$ldata)) 
			$form = 'Invoice saved';
		else $form = 'Error saving invoice!';
		print list_invoices($ldata,false);
	} else if ($form === 'Purchase time') {
		print purchase_time_form($ldata);
	} else {
		$form = "Voicemail Admin";
		print main_form($ldata);
	}
}

ll_disconnect(); # disconnect from lifeline db if you need to
