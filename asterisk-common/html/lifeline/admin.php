<?php
require_once("php/globals.php");
require_once("$lib/pw/auth.php");

$action = $_REQUEST['action'];
if ($action === 'logout') delete_login();

$ldata = login_response($_SERVER['PHP_SELF'],'ll_pw_data');
$vdata = ll_vendor($ldata['vid']);
$ldata = array_merge($ldata,$vdata);

if ($ldata['status'] == 'deleted') die("vendor {$ldata['vendor']} was deleted!");

require_once("php/forms.php");

if ($_REQUEST['listen']) {
	header("location: listen.php?from=admin&box={$_REQUEST['box']}");
} else if ($action === 'Manage account and users') {
	header("location: make.php?from=admin");
} else if ($action === 'Create box') {
	print create_new_box($ldata);
} else if ($action === 'Add time to box') {
	print update_box_time($ldata);
} else if ($action === 'Remove time from box') {
	print delete_months($ldata);
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
} else if ($action === 'Show boxes') {
	print view_boxes($ldata);
} else if ($action === 'Purchase time') {
	print confirm_purchase_form($ldata);
} else if ($action === 'Buy voicemail now') {
	print purchase_time($ldata);
} else if ($action === 'invoice') {
	print invoice($ldata);
} else if ($action === 'Search') {
	print view_boxes_form($ldata,ll_find_boxes($ldata['vid'],$_REQUEST['search']));
} else {
	$form = $_REQUEST['form'];
	if ($form === 'Create a new voicemail box') {
		print create_new_box_form($ldata);
	} else if ($form === 'Add time to an existing box' or $form === 'add') {
		print update_box_form($ldata);
	} else if ($form === 'sub') {
		print update_box_form($ldata,'Remove time from box');
	} else if ($form === 'del') {
		print update_box_form($ldata,'Delete box');
	} else if ($form === 'chsc') {
		print update_box_form($ldata,'Change security code');
	} else if ($form === 'edit') {
		print update_box_form($ldata,'Update name, email etc.');
	} else if ($form === 'View your voicemail boxes' or $form === 'find_boxes') {
		print find_boxes_form($ldata);
	} else if ($form === 'Show all invoices') {
		print list_invoices($ldata,true);
	} else if ($form === 'Show unpaid invoices') {
		print list_invoices($ldata,false);
	} else if ($form === 'Purchase time') {
		print purchase_time_form($ldata);
	} else {
		print main_form($ldata);
	}
}

ll_disconnect(); # disconnect from lifeline db if you need to
