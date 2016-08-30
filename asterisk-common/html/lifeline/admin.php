<?php
require_once("php/globals.php");
require_once("$lib/pw/auth.php");
require_once("twitterapi.php");
@include_once("news.php");
if (function_exists('gettweets')) $newsdiv = gettweets();

$action = $_REQUEST['action'];
if ($action === 'logout') delete_login();

require_once("php/ldata.php");
require_once("php/forms.php");

if ($_REQUEST['listen']) {
	header("location: listen.php?from=admin&box={$_REQUEST['box']}");
} else if ($action === 'Manage account and users') {
	header("location: make.php?from=admin");
} else if ($action === 'Create boxes') {
	print create_new_box($ldata);
	# this must happen after as create_new_box makes the boxlist
	if ($ldata['vid'] == POBOXVID) ll_sync_clients($_REQUEST['boxlist']);
} else if ($action === 'Create box') {
	if ($ldata['vid'] == POBOXVID) ll_sync_client();
	print create_new_box($ldata,$_REQUEST['box']);
} else if ($action === 'Really add time to box?' or $action === 'Really remove time from box?') {
	if ($ldata['vid'] == POBOXVID) ll_sync_client();
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
