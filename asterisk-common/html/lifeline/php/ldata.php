<?php
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
	header('location: /lifeline/clients.php');
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
<a href="clients.php?action=undelete_vendor&vid={$vdata['vid']}">undelete {$vdata['vendor']}</a>
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

