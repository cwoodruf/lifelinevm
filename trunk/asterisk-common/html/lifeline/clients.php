<?php
# like admin.php but with reduced functionality
# changes behaviour of forms such that pobox info is included
ini_set('display_errors',true);
error_reporting(E_ALL & ~E_STRICT & ~E_NOTICE);

require_once("php/globals.php");
$sitecolor = 'white';
require_once("$lib/pw/auth.php");
require_once("twitterapi.php");
@include_once("news.php");
if (function_exists('gettweets')) $newsdiv = gettweets();

$action = $_REQUEST['action'];
if ($action === 'logout') delete_login();

require_once("php/ldata.php");
require_once("php/forms.php");

if ($action === 'Create boxes' or $action === 'Really add time to box?' or $action === 'Really remove time from box?') {
	sync_client();
	if ($action === 'Create boxes') {
		print create_new_box($ldata,$_REQUEST['box']);
	} else if ($action === 'Really add time to box?' or $action === 'Really remove time from box?') {
		print update_box_time($ldata);
	}
} else if ($action === 'Confirm PO Box Update') {
	print update_pobox_time($ldata);
} else if ($action === 'Delete box') {
	print confirm_delete_form($ldata);
} else if ($action === 'Really delete box') {
	print delete_box($ldata);
} else if ($action === 'Change security code') {
	print confirm_update_seccode($ldata);
} else if ($action === 'Really change security code') {
	print update_seccode($ldata);
} else if ($action === 'Update name, email etc.') {
	sync_client();
	print update_personal($ldata);
} else if ($action === 'Update PO Box name, email etc.') {
	print update_pobox_personal($ldata);
} else if ($action === 'Show boxes') {
	print view_boxes($ldata);
} else {
	if ($form === 'Create a new voicemail box') {
		print create_new_box_form($ldata);
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
		$form = "Confirm voice mail box delete";
		print confirm_delete_form($ldata);	
	} else if ($form === 'chsc') {
		print update_box_form($ldata,'Change security code');
	} else if ($form === 'edit') {
		print update_box_form($ldata,'Update name, email etc.');
	} else if ($form === 'edit_pobox' or $form === 'Edit PO Box') {
		print update_pobox_form($ldata,'Update PO Box name, email etc.');
	} else if ($form === 'Search Boxes' or $form === 'Search Clients') {
		print find_boxes_form($finddata,ll_find_clients($findvid,$_REQUEST['search']));
	} else if ($form === 'Print PO Box Reminders') {
		print pobox_reminders_form($ldata);
	} else {
		$form = "Client Admin";
		print main_clients_form($ldata);
	}
}

ll_disconnect(); # disconnect from lifeline db if you need to

# update other tables related to a voicemail box
function sync_client() {
	global $vdata;
	$personal = get_client();
	$cid = $personal['cid'];
	# the cid is necessary here to glue together the other records
	if ($cid > 0) {
		$pers = $personal;
		# do not overwrite existing notes in the poboxes record
		unset($pers['notes']);
		if ($_REQUEST['updatepoboxes']) {
			foreach ($_REQUEST['poboxes'] as $pobox) {
				if (ll_pobox_ok($pobox) != null) {
					ll_pobox_update_personal($vdata, $pobox, $pers);
					ll_pobox_update_months($vdata, $pobox, $_REQUEST['months']);
				}
			}
		} else if (is_array($pers) and 
				($pobox = ll_pobox_ok($_REQUEST['pobox'])) != null) {
			ll_pobox_update_personal($vdata, $pobox, $pers);
			ll_pobox_update_months($vdata, $pobox, $_REQUEST['months']);
		}
	}
	$_REQUEST['personal'] = $personal;
	return $cid;
}

# get or create a client record using the cid or name
function get_client() {
	global $vdata;
	# use the name, notes fields to update the clients and poboxes tables
	# use the vm box as the basis for this
	if (!isset($_REQUEST['personal'])) {
		$cid = $_REQUEST['cid'];
		$personal = ll_box($_REQUEST['box']);
		unset($personal['paidto']);
	} else {
		$cid = $_REQUEST['personal']['cid'];
		$personal = $_REQUEST['personal'];
	}
	# find or create a clients record
	if (preg_match('#^\d+$#', $cid)) {
		$cdata = ll_client($cid);
		if (!$cdata) $cid = null;
		else {
			$personal['cid'] = $cid;
			ll_client_update($vdata, $personal);
		}
	} else $cid = null;
	# with no explicit cid we should check the name first
	# then make a new record if the name can't be found
	if ($cid == null) {
		if (isset($personal['name'])) {
			$cdata = ll_client_from_name($personal['name']);
			if ($cdata === false or !isset($cdata['cid'])) {
				$cid = ll_client_insert($vdata, $personal);
			} else {
				$cid = $cdata['cid'];
				$personal['cid'] = $cid;
				ll_client_update($vdata, $personal);
			}
		}
	}
	# if we get to this point we should have a cid
	$personal['cid'] = $cid;
	$personal['vid'] = $vdata['vid'];
	return $personal;
}

