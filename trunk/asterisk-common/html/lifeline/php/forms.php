<?php
require_once("$lib/asterisk.php");
require_once("php/lifeline-schema.php");
$back = "<a href=/lifeline/admin.php>Back to admin</a>";
if ($permcheck['logins'])
	$manage = "<a href=\"admin.php?action=Manage account and users\">Manage account and users</a>";
$table = table_header();

function table_header($cp=5,$cs=0,$b=0,$w=400,$style='') {
	return "<table cellpadding=$cp cellspacing=$cs border=$b width=$w $style>";
}

function head() {
	global $form;
	$title = empty($form) ? "main page" : $form;
	return <<<HTML
<html>
<head>
<title>Voicemail Admin: $title</title>
<link rel=stylesheet type=text/css href=/lifeline/css/admin.css>
</head>
<body bgcolor=lightyellow>
HTML;
}

function form_top($data,$show_goback=true,$show_status=true,$method='get',$formjs='') {
	global $back, $manage;
	global $vend, $ldata;
	global $form;
	$login = $data['login'];
	$vendor = $data['vendor'];
	$app = $data['app'];
	$time = date('r',$data['time']);
	$vend = ll_vendor($data['vid']);
	if ($show_status) $status = vend_status_str($vend);
	$goback = $show_goback ? "$manage - $back" : '';
	if ($ldata) {
		$logout = <<<HTML
<nobr>
<a href="admin.php?action=logout">Log out</a>
HTML;
		if (preg_match('#^\d+$#', $ldata['orig_vid']) and $ldata['orig_vid'] == $ldata['initial_vid']) {
			$logout .= <<<HTML
&nbsp; 
/ &nbsp; <a href="admin.php?switch_vendor={$ldata['orig_vid']}">
Return to {$ldata['orig_vendor']}</a>
HTML;
		}
		$logout .= "</nobr>\n";
	}
	$head = head();
	$search_form = search_form($data);
	return <<<HTML
$head
<center>
<h4>Vendor: $vendor <span style="font-weight: normal;">$goback &nbsp;&nbsp; $logout</span></h4>
$status<p>
$search_form
<h3>$form</h3>
<form action="{$_SERVER['PHP_SELF']}" name="topform" id="topform" method="$method" $formjs>
HTML;
}

function form_end($data) {
	global $adminfo;
	$uptime = preg_replace('#,?\s*\d+ seconds.*#','',get_uptime());
	return <<<HTML
</form>
<p>
<span class="support">
We are on twitter: <a class="support" href="http://twitter.com/lifelinevm">lifelinevm</a>
The Lifeline voice mail system is powered by <a class="support" href="http://asterisk.org/">Asterisk</a>. 
$uptime.
$adminfo
</span>
</center>
</body>
</html>
HTML;
}

function credit_left($vid) {
	$vend = ll_vendor($vid);
	$cl = $vend['credit_limit'];
	$m = $vend['unpaid_months'];
	if ($cl < 0) return true; 
	return $cl - $m;
}

function main_form($data) {
	global $ldata, $permcheck, $min_purchase;

	$top = form_top($data,false); 
	$end = form_end($data);
	if ($permcheck['invoices']) {
		$vend = ll_vendor($ldata['vid']);
		$credit = credit_left($ldata['vid']);
		if ($vend['credit_limit'] >= 0) {
			$limit = "Your credit limit is {$vend['credit_limit']} months.<br>";
			if ($credit != 1) $s = 's';
			if ($credit < 0) {
				$remaining = "You are over your credit limit by ".
						(abs($credit))." month$s.<br>";
			} else {
				$remaining = "You can purchase $credit month$s more voicemail.";
			}
		}
		if ($credit >= $min_purchase) {
			$purchase = <<<HTML
<input type=submit name=form value="Purchase time">
<br>
$limit
$remaining
HTML;
		} else {
			$purchase = <<<HTML
<em>Please pay any outstanding invoices before buying more voicemail.</em>
<br>
$limit
$remaining
HTML;
		}
		$purchase .= <<<HTML
<br>
<nobr>
<input type=submit name=form value="Show all invoices"> 
<input type=submit name=form value="Show unpaid invoices">
</nobr>
<p>
HTML;
	}
	if ($ldata['months'] > 0) {
		$addtime_buttons = <<<HTML
<input type=submit name=form value="Create a new voicemail box"> <p>
<input type=submit name=form value="Add time to an existing box"> <p>
HTML;
	}

	if (preg_match('#logins#',$data['perms'])) 
		$users = '<input type=submit name=action value="Manage account and users"> <p>';
	return <<<HTML
$top
$addtime_buttons
<input type=submit name=form value="View payments"> 
<input type=submit name=form value="View call events">
<p>
$users
$purchase
$end
HTML;
}

function vend_status_str($vend) {
	global $min_purchase, $permcheck;
	if ($vend['months'] == 1) {
		$months = "1 month";
	} else if ($vend['months'] == 0) {
		$months = "no";
		if ($permcheck['invoices']) {
			$purchaselink = "<a href=\"admin.php?form=Purchase time\">Purchase time.</a>";
		}
	} else {
		$months = $vend['months']." months";
	}
	if ($vend['actual_months'] == 1) $acts = '';
	else $acts = 's';
	$status = <<<HTML
{$vend['vendor']} has $months voice mail available. 
$purchaselink
<br>
HTML;
	return $status;
}
 
function create_new_box_form($data) {
	global $vend;
	$formjs = <<<JS
onsubmit="
if (boxes.value > 1) 
	return confirm('Create '+boxes.value+' voice mail boxes? Action cannot be undone.');
else return confirm('Create voice mail box? Action cannot be undone.');
"
JS;
	# if this is a redirect from the old gc.pl file we'll have some paycode data
	$pc = ll_paycodeinfo($_REQUEST['paycode']);
	$max = MAXBOXES;
	if (MAXMONTHS > $vend['months']) $maxmonths = $vend['months'];
	else $maxmonths = MAXMONTHS;
	if (is_array($pc)) {
		$months = $pc['months'];
		$vend['paycode'] = $_REQUEST['paycode'];
		$boxeswidget = <<<HTML
Number of boxes to create: &nbsp; 1 <input type=hidden name=boxes value=1>  &nbsp;&nbsp; 
HTML;
	} else {
		$months = 1;
		$boxeswidget = <<<HTML
Number of boxes to create &nbsp; <input size=3 name=boxes value=1> (maximum $max) &nbsp;&nbsp; 
HTML;
	}
	$top = form_top($data,true,true,'post',$formjs); 
	$end = form_end($data);
	$trans = ll_generate_trans($vend,'boxes');
	$personal = mk_personal_input(null,$vend);
	$payment_form = payment_form($box);
	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
$boxeswidget
Valid for &nbsp; <input size=3 name=months value="$months"> &nbsp; months (up to $maxmonths months). &nbsp;&nbsp; 
$personal
$payment_form
<br>
<input type=submit name=action value="Create boxes" class=action>
$end
HTML;
}

/*
mysql> desc payments;
+--------+-------------+------+-----+---------+-------+
| Field  | Type        | Null | Key | Default | Extra |
+--------+-------------+------+-----+---------+-------+
| box    | varchar(32) | NO   | PRI |         |       | 
| vid    | int(11)     | NO   | PRI | 0       |       | 
| paidon | datetime    | NO   | PRI | NULL    |       | 
| amount | float       | YES  |     | 0       |       | 
| hst    | float       | YES  |     | 0       |       | 
| months | int(11)     | YES  |     | 0       |       | 
| login  | varchar(32) | NO   | PRI |         |       | 
| notes  | text        | YES  |     | NULL    |       | 
+--------+-------------+------+-----+---------+-------+
*/
function payment_form($box) {
	global $ldata,$vend;
	if (!preg_match('#^\d+$#',$box)) $box = "";

	$payment = $_REQUEST['payment'];
	if (!is_array($payment)) $payment = array();

	$now = $payment['paidon'];
	if (!preg_match('#^\d{4}-\d{2}-\d{2}#',$now)) $now = date('Y-m-d H:i:s');

	$amount = $payment['amount'];
	if (!is_numeric($amount) or $amount < 0) $amount = sprintf('%.2f', 0);

	$notes = htmlentities($payment['notes']);

	if ($box) $forbox = "for $box";

	return <<<HTML
<h4>Payment info $forbox (leave as is if not needed)</h4>
<table cellpadding=5 cellspacing=0 border=0>
<tr><td>Paid On</td><td><input size=20 name="payment[paidon]" value="$now"></td></tr>
<tr><td>Amount Paid</td>
<td>
<input size=10 name="payment[amount]" value="$amount"></td></tr>
<tr><td>Notes</td><td><input size=40 name="payment[notes]" value="$notes"></td></tr>
</table>

HTML;
}

function create_new_box($data) {
	global $ldata, $phone;
	$vend = ll_vendor($data['vid']);
	$trans = ll_valid_trans($_REQUEST['trans']);
	ll_delete_trans($vend,$trans,'new');

	$boxes = $_REQUEST['boxes'];
	if (!preg_match('#^\d\d?$#',$boxes) or $boxes <= 0) die("create_new_box: invalid number of boxes");
	if ($boxes > MAXBOXES) die("please select a smaller number of boxes than ".MAXBOXES);

	$months = $_REQUEST['months'];
	if (!preg_match('#^\d\d?$#',$months) or $months <= 0) die("create_new_box: invalid number of months");
	if ($months > MAXMONTHS) die("please select a smaller number of months than ".MAXMONTHS);

	$totalmonths = $months * $boxes;
	if ($vend['months'] < $totalmonths) die("you only have $totalmonths months available!");
	$netmonths = $vend['months'];
	$vid = $vend['vid'];

	$llphone = $_REQUEST['personal']['llphone'];
	if (empty($llphone)) $llphone = $phone;

	list($min_box,$max_box) = get_box_range(); # pick a random box range 
	if ($boxes == 1) {
		list ($box,$seccode,$paidto) = ll_new_box($trans,$vend,$months,$llphone,$min_box,$max_box);
		$amount = ll_update_payment($box,$vid,$ldata['login'],$months,$_REQUEST['payment']);
		ll_update_personal($vend,$box,$_REQUEST['personal']);

		$netmonths -= $months;
		ll_save_to_table('update','vendors',array('months' => $netmonths),'vid',$vid);

		return new_box_instructions($data,$box,$seccode,$amount,$_REQUEST['personal']);
	}
	$boxlist = array();
	for ($i=0; $i < $boxes; $i++) {
		list ($box,$seccode,$paidto) = ll_new_box($trans,$vend,$months,$llphone,$min_box,$max_box);
		$amount = ll_update_payment($box,$vid,$ldata['login'],$months,$_REQUEST['payment'],$boxes);
		ll_update_personal($vend,$box,$_REQUEST['personal']);

		$netmonths -= $months;
		ll_save_to_table('update','vendors',array('months' => $netmonths),'vid',$vid);

		$bdata = $_REQUEST['personal'];
		$bdata['box'] = $box;
		$bdata['seccode'] = $seccode;
		$bdata['paidto'] = $paidto;
		$bdata['months'] = $months;
		$bdata['amount'] = $amount;
		$boxlist[] = $bdata;
	}
	return show_boxes($data,$boxlist);
}

function showcode($data,$box,$seccode,$format) {
	if (preg_match('#^\d{4}$#',$seccode)) return $seccode;

	if (($cracked = ll_showcode($seccode)) === false) $cracked = "unavailable";
	else $cracked = sprintf('%04d',$cracked);

	$head = head();
	if ($format == 'html') {
		return <<<HTML
$head
<center>
<h4>box $box uses security code $cracked</h4>
</center>
</body>
</html>
HTML;
	}
	return $cracked;
}

function show_boxes($data,$boxlist) {
	global $lib;
	$top = form_top($data); 
	$end = form_end($data);
	
	$html = <<<HTML
$top
<h4>Voice mail boxes</h4>
<i>If you use <a href="http://mozilla.com">Firefox</a> 
you may find <a href="http://dafizilla.sourceforge.net/table2clip/">Table2Clipboard</a>
helpful for copying this information into a spreadsheet</i>
<p>
<style>th { font-weight: normal; }</style>
<table cellpadding=5 cellspacing=0 border=0>
<tr><th>box</th><th>security code</th><th>subscription</th><th>amount</th><th>notes</th></tr>

HTML;
	foreach ($boxlist as $bdata) {
		$box = $bdata['box'];
		$seccode = $bdata['seccode'];
		if (strlen($seccode) >= 32) {
			$seccode = showcodelink($box,$seccode);
		}

		if ($bdata['months']) {
			$months = $bdata['months'].' months';
		} else if ($bdata['status']) {
			$months = $bdata['status'];
		} else if (!preg_match('#0000-00-00#',$bdata['paidto'])) {
			$months = $bdata['paidto'].'&nbsp;';
		}
		$amount = $bdata['amount'] > 0 ? sprintf('%.2f',$bdata['amount']) : "";
		$notes = $bdata['name'].'&nbsp;'.$bdata['email'].' '.$bdata['notes'];

		$html .= <<<HTML
<tr>
<th><a href="admin.php?form=Search Boxes&search=$box">$box</a></th>
<th>$seccode</th>
<th>$months</th>
<th>$amount</th>
<th>$notes</th>
</tr>
HTML;
	}
	$html .= "</table>\n$end";
	return $html;
}

function showcodelink($box,$seccode) {
	$box = htmlentities($box);
	$seccode = htmlentities($seccode);
	$showcode = "admin.php?box=$box&seccode=$seccode&form=showcode";
	return <<<HTML
<a href="javascript: void(0);" 
   onclick="window.open(
	'$showcode','showcode',
	'top=300,left=300,locationbar=no,statusbar=no,menubar=no,scrollbars=no,height=100,width=300,statusbar=no');"
>show</a>
HTML;
}

function new_box_instructions($data,$box,$seccode,$amount,$personal) {
	global $table;

	$vend = ll_vendor($data['vid'],true);

	$top = form_top($data); 
	$end = form_end($data);
	if ($amount > 0.0) $amount = sprintf('%.2f',$amount);
	else $amount = '&nbsp;';

	$bdata = ll_box($box);
	if ($bdata['paidto'] != '0000-00-00' and preg_match('#^\d{4}-\d{2}-\d{2}$#',$bdata['paidto'])) 
		$paidto = "paid to {$bdata['paidto']}";
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td>$box</td></tr>
<tr><td><b>Status:</b></td><td>{$bdata['status']} $paidto</td></tr>
<tr><td><b>Name:</b></td><td>{$personal['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$personal['email']}</td></tr>
<tr><td><b>Amount:</b></td><td>{$amount}</td></tr>
<tr><td><b>Notes:</b></td><td>{$personal['notes']}</td></tr>
<tr><td colspan=2 align=right>
<b>PRINT THIS:</b>
<a href="index.php?box=$box&seccode={$bdata['seccode']}&amount=$amount&llphone={$bdata['llphone']}" 
   target=_blank>receipt / instructions</a>
</td></tr>
</table>
$instr
$end
HTML;
}

function update_box_form($data,$action="Add time to box") {
	global $_REQUEST;
	global $vend;
	global $form;
	global $table;
	$form = $action;
	$submittype = 'action';
	$box = $_REQUEST['box'];
	if (preg_match('#^\d+$#',$box)) $bdata = ll_box($box);
	if (preg_match('#^2\d\d\d#',$bdata['paidto'])) {
		$paidto = preg_replace('# .*#','',$bdata['paidto']);
		$status = "(Paid to $paidto";
		if ($bdata['status']) $status .= ", status {$bdata['status']}";
		$status .= ")";
	} else if (preg_match('#add (\d+) month#', $bdata['status'], $m)) {
		$today = date('Y-m-d');
		$months = $m[1];
		$s = $months == 1 ? '': 's';
		$status = <<<HTML
<div style="margin-top: 10px; width: 80%; padding: 3px; border: 1px black solid">
<b>Set start date</b> (YYYY-MM-DD)
<input name=startdate size=10 maxsize=10> 
<a href="javascript:void(0);" onclick="topform.startdate.value='$today'; return false;">
click here to use today's date</a> 
<br>
<i>Subscription date will be set to $months month$s from this date.</i> 
</div>
HTML;
	}
	if ($box != '') {
		$is_hidden = 'type=hidden';
	}
	$top = form_top($data); 
	$end = form_end($data);
	if ($action === 'Change security code') {
		$seccode_input = " New security code: <input size=4 name=seccode> &nbsp;&nbsp; ";
	} else if ($action === 'Update name, email etc.') {
		$edit_input = mk_personal_input($bdata);
	} else if (preg_match('#\btime\b#',$action)) {
		if ($action === 'Remove time from box') {
			$remove = 1;
			$monthsleft = ll_box_months_left($bdata);
			if ($bdata['canremove'] < $monthsleft) $monthsleft = $bdata['canremove'];
			if ($monthsleft <= 0) 
				return "$top<h3>Box $box has less than one month left on its subscription.</h3>$end";
			$submittype = 'form';
			$nextsubmit = 'Really remove time from box?';
			$maxmonths = $monthsleft;
		} else {
			$remove = 0;
			$months = 1;
			$submittype = 'form';
			$nextsubmit = 'Really add time to box?';
			if ($vend['months'] > 0) {
				if ($vend['months'] < MAXMONTHS) $maxmonths = $vend['months'];
				else $maxmonths = MAXMONTHS;
			}
		}
		$month_input = <<<HTML
Months: <input size=3 name=months value=$months> (up to $maxmonths months)
HTML;
	}
	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
<input type=hidden name=nextsubmit value="$nextsubmit">
<input type=hidden name=remove value="$remove">
<div style="width: 740px; background: cornsilk; border: 1px black solid; padding: 3px;">
Box: <input $is_hidden name=box size=7 value="$box"><b>$box</b> $status &nbsp;&nbsp;
$seccode_input
$month_input
$edit_input
</div>
<br>
<input type=submit name=$submittype value="$action" class=action>
$end
HTML;
}

function mk_personal_input($bdata=array(),$vend=null) {
	global $ldata,$table,$phone,$ll_host;
	if (!is_array($vend) and $bdata['vid']) $vend = ll_vendor($bdata['vid']);
	if (empty($bdata['notes'])) $bdata['notes'] = $vend['vendor'].' '.$vend['paycode'];
	$myphone = $bdata['llphone'];
	if (empty($myphone)) $myphone = $vend['llphone'];
	if (empty($myphone)) $myphone = $phone;
	$phones = explode(':',$myphone);
	if (count($phones) > 1) {
		# this is mainly for jobwave / triumph	
		$vanpat = '#vancouver|burnaby|surrey|langely|coquitlam|maple\s*ridge|richmond#';
		$tollfreepat = '#^\s*(1\s*|)8\d\d#';
		$phonesel = "<select name=\"personal[llphone]\">";
		$islocal = preg_match($vanpat, $ldata['login']);
		foreach ($phones as $ph) {
			if (!$islocal and preg_match($tollfreepat, $ph)) {
				$selected = 'selected';
			} else if ($islocal and !preg_match($tollfreepat,$ph)) {
				$selected = 'selected';
			} else {
				$selected = '';
			}
			$phonesel .= "<option $selected>$ph\n";
		}
		$phonesel .= "</select>\n";
	} else {
		$phonesel = "$myphone <input type=hidden name=\"personal[llphone]\" value=\"$myphone\">";
	}
	return <<<HTML
<p>
$table
<tr><td>Phone:</td><td>$phonesel</td></tr>
<tr><td>Name:</td><td><input size=32 name="personal[name]" value="{$bdata['name']}"></td></tr>
<tr><td>Email:</td><td><input size=40 name="personal[email]" value="{$bdata['email']}"></td></tr>
<tr><td>Notes:</td><td><input size=64 name="personal[notes]" value="{$bdata['notes']}"></td></tr>
</table>

HTML;
}

function box_activity($data) {
	global $ldata;
	$box = $_REQUEST['box'];
	if (!preg_match('#^\d+$#',$box)) die("showactivity: invalid box!");
	if (ll_has_access($ldata,$box)) die("showactivity: you do not have access to box $box!");
	$top = form_top($data); 
	$end = form_end($data);
	$payments = paymentlist($box,$data['vid']);
	$callhtml = callhtml($box);
	return <<<HTML
$top
$payments
$callhtml
$end
HTML;
}

function datesel($vid,$selected,$callback) {
	$dates = $callback($vid);
	$sel = "<select name=date>\n";
	foreach ($dates as $date) {
		if ($selected == $date) $s = "selected";
		else $s = "";
		$sel .= "<option $s>$date</option>\n";
	}
	$sel .= "</select>\n";
	return $sel;
}

function paymentlist($box,$vid) {
	$payments = ll_get_payments($box,$vid);
	if (!count($payments)) return;
	$title = "<h4>Payments or changes for box $box</h4>";
	return formatpaymentlist($title, $payments);
}

function vendlink($vid) {
	global $permcheck;
	if ($permcheck['vendors']) {
		$vendlink = "<a href=\"make.php?from=admin&findvid=$vid\">$vid</a>";
	} else {
		$vendlink = $vid;
	}
	return $vendlink;
}

function formatpaymentlist($title,$payments) {
	global $permcheck;
	$html = <<<HTML
$title
<table cellpadding=5 cellspacing=0 border=1>
<tr>
<th>#</tH><th>Box</th><th>Date</th>
<th>Amount</th><th>Tax</th><th>Months</th>
<th>Vendor</th><th>Login</th><th>Notes</th>
</tr>

HTML;
	foreach ($payments as $p) {
		$item++;
		$amount = sprintf('$ %.2f', $p['amount']);
		$hst = sprintf('$ %.2f', $p['hst']);
		$vendlink = vendlink($p['vid']);
		$html .= <<<HTML
<tr valign=top>
<td>$item</td>
<td><a href="admin.php?form=Search Boxes&search={$p['box']}">{$p['box']}</a></td>
<td><nobr>{$p['paidon']}</nobr></td>
<td align=right><nobr>$amount</nobr></td>
<td align=right><nobr>$hst</nobr></td>
<td align=right>{$p['months']} &nbsp;</td>
<td>{$p['vendor']} &nbsp;</td>
<td>{$p['login']} &nbsp;</td>
<td>{$p['notes']} &nbsp;</td>
</tr>

HTML;
	}
	$html .= "</table>\n";
	return $html;
}

function callhtml($box,$limit=50) {
	$calls = ll_calls($box,$limit);
	$box = htmlentities($box);
	if (!count($calls)) return "<h4>No activity for box $box</h4>";
	return formatcallhtml("<h4>Recent login attempts and messages for box $box</h4>", $calls);
}

function formatcallhtml($title,$calls) {
	$callhtml = <<<HTML
$title
<table cellpadding=5 cellspacing=0 border=1>
<tr><th>#</th><th>Box</th><th>Vendor Id</th><th>Time</th><th>Action</th><th>Caller ID</th></tr>

HTML;
	foreach ($calls as $call) {
		if ($call['action'] == 'll-flagmsg.pl') $calltype = 'message left';
		else if ($call['action'] == 'll-login.pl') $calltype = "login: status {$call['status']}";
		else if ($call['action'] == 'll-saveseccode.pl') 
			$calltype = "change security code";
		else $calltype = $call['action']." ".$call['status'];
		$vendlink = vendlink($call['vid']);
		$callerid = htmlentities($call['callerid']);
		$i++;
		$callhtml .= <<<HTML
<tr>
<td>$i</td>
<td><a href="admin.php?form=Search Boxes&search={$call['box']}">{$call['box']}</a></td>
<td>$vendlink</td>
<td>{$call['call_time']}</td>
<td>$calltype &nbsp;</td>
<td>$callerid &nbsp;</td>
</tr>
HTML;
	}
	$callhtml .= "</table>\n";
	return $callhtml;
}

function delete_months($data) {
	global $_REQUEST;
	return update_box_time($data,(-1 * $_REQUEST['months']));
}

function confirm_delete_form($data) {
	global $_REQUEST;
	$box = $_REQUEST['box'];
	ll_check_box($box);
	$top = form_top($data);
	$end = form_end($data);
	return <<<HTML
$top
<input type=hidden name=box value="$box">
Delete box $box? &nbsp;&nbsp; <input type=submit name=action value="Really delete box">
$end
HTML;
}

function delete_box($data) {
	global $_REQUEST;
	$box = $_REQUEST['box'];
	$vend = ll_vendor($data['vid'],true);
	ll_delete_box($vend,$box);
	cleanup_files($box);
	$vend = ll_vendor($data['vid'],true);
	$top = form_top($data); 
	$end = form_end($data);
	return <<<HTML
$top
Box $box now deleted.
$end
HTML;
}

function cleanup_files($box) {
	global $asterisk_lifeline;
	global $asterisk_rectype;
	global $asterisk_deleted;
	$dir = "$asterisk_lifeline/$box/messages";
	if (($dh = @opendir($dir)) === false) return;
	$greeting = "$dir/greeting.$asterisk_rectype";
	$deleted = "$dir/greeting.$asterisk_deleted.$asterisk_rectype";
	if (is_file($greeting)) {
		@unlink($deleted);
		rename($greeting,$deleted);
	}
	while (($item = readdir($dh)) !== false) {
		$name = "$dir/$item";
		if (!is_file($name)) continue;
		if (!preg_match("#(.*)\.($asterisk_rectype)$#",$name,$m)) continue;
		$deleted = $m[1].".$asterisk_deleted.".$m[2];
		@unlink($deleted);
		rename($name,$deleted);
	}
}

function confirm_update_seccode($data) {
	global $_REQUEST;
	$box = $_REQUEST['box'];
	ll_check_box($box);
	$seccode = $_REQUEST['seccode'];
	ll_check_seccode($seccode);
	$top = form_top($data);
	$end = form_end($data);
	return <<<HTML
$top
<input type=hidden name=box value="$box">
<input type=hidden name=seccode value="$seccode">
Change security code for box $box to $seccode? &nbsp;&nbsp; 
<input type=submit name=action value="Really change security code">
$end
HTML;
}

function update_seccode($data) {
	global $_REQUEST;
	global $vend;
	$top = form_top($data);
	$end = form_end($data);
	$box = $_REQUEST['box'];
	$seccode = $_REQUEST['seccode'];
	ll_update_seccode($vend,$box,$seccode);
	return <<<HTML
$top
Security code for box $box updated to $seccode.
$end
HTML;
}
function confirm_update_box_time($data,$months='') {
	global $table,$ldata;

	$box = $_REQUEST['box'];
	if (empty($box)) die("need a box number!");

	if ($months === '') $months = $_REQUEST['months'];
	if (empty($months)) $months = 0;
	$months = $_REQUEST['remove'] ? -1 * abs($months) : abs($months);

	$vend = ll_vendor($data['vid']);
	$vend = ll_vendor($data['vid'],true);

	$bdata = ll_box($box);
	if (empty($bdata)) die("box $box does not exist!");

	$boxupdate = ll_check_time($vend,$box,$months);
	$trans = ll_generate_trans($vend,'boxes');

	$top = form_top($data); 
	$end = form_end($data);
	$payment_form = payment_form($box);
	if ($bdata['vid'] != $vend['vid']) {
		$original = 'Original ';
		# if we are the parent don't do anything otherwise change the vendor id
		if (!ll_isparent($vend['vid'], $bdata)) {
			$newvendfield = <<<HTML
<tr>
<td><b>New Vendor:</b></td>
<td>
<u>{$vend['vendor']}</u>
</td>
</tr>
HTML;
		}
	}

	$nextsubmit = htmlentities($_REQUEST['nextsubmit']);
	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
<table cellpadding=3 cellspacing=0 border=0>
<tr><td><b>Box:</b></td><td><input type=hidden name=box value="$box"><b>$box</b></td></tr>
<tr><td><b>Months:</b></td><td><input type=hidden name=months value="$months"><b>$months</b></td></tr>
<tr><td><b>{$original}Vendor:</b></td><td>{$bdata['vendor']}</td></tr>
$newvendfield
<tr><td><b>Name:</b></td><td>{$bdata['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$bdata['email']}</td></tr>
<tr><td><b>Notes:</b></td><td>{$bdata['notes']}</td></tr>
<tr><td><b>Paid to:</b></td><td>{$bdata['paidto']}</td></tr>
<tr><td><b>Status:</b></td><td>{$bdata['status']}</td></tr>
</table>
$payment_form
<br>
<input type=submit name=action value="$nextsubmit" class=action>
$end
HTML;
}

function update_box_time($data,$months='') {
	global $table,$ldata;
	$vend = ll_vendor($data['vid']);
	if ($months === '') $months = $_REQUEST['months'];

	$box = $_REQUEST['box'];
	ll_delete_trans($vend,$_REQUEST['trans'],$box);

	$bdata = ll_box($box);
	if (empty($bdata)) die("box $box can't be updated because it doesn't exist!");

	$amount = ll_update_payment($box,$data['vid'],$ldata['login'],$months,$_REQUEST['payment']);

	if ($bdata['vid'] != $vend['vid']) {
		# if we are the parent don't do anything otherwise change the vendor id
		if (ll_isparent($vend['vid'], $bdata)) {
			$vid = $bdata['vid'];
		} else {
			# only allow adding time when we don't own the box
			if ($months < 0) 
				die("Months should be greater than zero not $months!");
			$vid = $vend['vid'];
		}
	} else {
		$vid = $vend['vid'];
	}

	if (is_numeric($months)) {
		$paidto = ll_add_time($vid,$box,$months);
		$bdata = ll_box($box,true);
		$vend = ll_vendor($data['vid'],true);
	}
		
	$top = form_top($data); 
	$end = form_end($data);
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td><a href="admin.php?form=Search Boxes&search=$box">$box</a></td></tr>
<tr><td><b>Paid to:</b></td><td>$paidto</td></tr>
<tr><td><b>Status:</b></td><td>{$bdata['status']}</td></tr>
<tr><td><b>Vendor:</b></td><td>{$bdata['vendor']}</td></tr>
<tr><td><b>Amount paid:</b></td><td>$amount</td></tr>
<tr><td colspan=2 align=right>
<b>PRINT THIS:</b>
<a href="index.php?box=$box&seccode={$bdata['seccode']}&amount=$amount&llphone={$bdata['llphone']}" 
   target=_blank>receipt / instructions</a>
</td></tr>
</table>
$end;
HTML;
}

function update_personal($data) {
	global $table, $vend,$ldata;
	$top = form_top($data);
	$end = form_end($data);
	$box = $_REQUEST['box'];
	if (!preg_match('#^\d+$#',$box)) 
		die("update_personal: box should be a number not $box!");

	ll_update_personal($vend,$box,$_REQUEST['personal']);
	if (preg_match('#^\s*(2\d\d\d-\d\d-\d\d)#',$_REQUEST['startdate'],$m)) {
		$startdate = $m[1];
		ll_set_paidto($box,$startdate);
		$notes = "Start date added";
	} else {
		$notes = "Edited personal data";
	}

	$amount = ll_update_payment($box,$data['vid'],$ldata['login'],($months=0),
		array('paidon' => date('Y-m-d H:i:s'),'amount' => 0, 'notes' => $notes)
	);

	$bdata = ll_box($box,($refresh=true));
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td><a href="admin.php?form=Search Boxes&search=$box">$box</a></td></tr>
<tr><td><b>Name:</b></td><td>{$bdata['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$bdata['email']}</td></tr>
<tr><td><b>Notes:</b></td><td>{$bdata['notes']}</td></tr>
<tr><td><b>Status:</b></td><td>{$bdata['status']}</td></tr>
<tr><td><b>Paid to:</b></td><td>{$bdata['paidto']}</td></tr>
<tr><td colspan=2 align=right>
<b>PRINT THIS:</b>
<a href="index.php?box=$box&seccode={$bdata['seccode']}&llphone={$bdata['llphone']}" 
   target=_blank>instructions</a>
</td></tr>
</table>
$end

HTML;
}

function find_calls_form($data) {
	$date = $_REQUEST['date'];
	if (!preg_match('#^\d{4}-\d{2}-\d{2}$#',$date)) 
		$date = date('Y-m-d');
	$calls = ll_calls_by_date($data['vid'],$date);
	$numcalls = count($calls);
	$top = form_top($data);
	$end = form_end($data);
	if ($numcalls == 0) {
		$callhtml = "<h4>No call events for $date</h4>";
	} else {
		if ($numcalls == 1) $s = '';
		else $s = 's';
		$title = "<h4>$numcalls call event$s for $date</h4>";
		$callhtml = formatcallhtml($title, $calls);
	}
	$datesel = datesel($data['vid'],$date,'ll_calldates');
	return <<<HTML
$top
$datesel <input type=submit name=form value="View call events">
<p>
$callhtml
$end
HTML;
}

function find_payments_form($data) {
	$date = $_REQUEST['date'];
	if (!preg_match('#^\d{4}-\d{2}-\d{2}$#',$date)) 
		$date = date('Y-m-d');
	$payments = ll_payments_by_date($data['vid'],$date);
	$numpayments = count($payments);
	$top = form_top($data);
	$end = form_end($data);
	if ($numpayments == 0) {
		$paymentlist = "<h4>No changes for $date</h4>";
	} else {
		if ($numpayments == 1) $s = '';
		else $s = 's';
		$title = "<h4>$numpayments change$s for $date</h4>";
		$paymentlist = formatpaymentlist($title, $payments);
	}
	$datesel = datesel($data['vid'],$date,'ll_paymentdates');
	return <<<HTML
$top
$datesel <input type=submit name=form value="View payments">
<p>
$paymentlist
$end
HTML;
}

function find_boxes_form($data,$boxes=null) {
	global $table;
	$top = form_top($data); 
	$end = form_end($data);
	$showkids = $_REQUEST['showkids'] ? true : false;
	if (!is_array($boxes)) $boxes = ll_boxes($data,$showkids,'not_deleted','order by box');
	$truncdate = '#-\d+(?:| .*)$#';
	$html = <<<HTML
$top
<h4>Voice mailboxes</h4>
HTML;
	$html .= view_boxes_form($data,$boxes);
	return $html;
}

function mk_sel($name,$items,$multiple=null) {
	if (!is_array($items)) return;
	if ($multiple == 'multiple') $size = count($items) > 10 ? 10 : count($items);
	$select = "<select name=\"$name\" $multiple $size>\n<option>\n";
	foreach ($items as $item) {
		if ($item) $select .= "<option>$item\n";
	}
	$select .= "</select>\n";
	return $select;
}

function search_form($data) {
	$search = htmlentities($_REQUEST['search']);
	if (!$search and $_REQUEST['box']) $search = htmlentities($_REQUEST['box']);
	return <<<HTML
<form action=admin.php method=get>
<input name="search" value="$search">
<input type=hidden name="vid" value="{$data['vid']}">
<input type=submit name=form value="Search Boxes">
<br>
Boxes: &nbsp;
<a href="admin.php?form=Search Boxes&search=add [0-9]* months&vid={$data['vid']}">Show unused</a> &nbsp;&nbsp;
<a href="admin.php?form=Search Boxes&search=-deleted">Show active</a> &nbsp;&nbsp;
<a href="admin.php?form=Search Boxes&search=">Show all</a> &nbsp;&nbsp;
<a href="admin.php?form=Search Boxes&search=deleted&vid={$data['vid']}">Show deleted</a> &nbsp;&nbsp;
</form>
HTML;

}

function view_boxes_form($data,$boxes=null) {
	global $table;
	global $vend,$permcheck;
	$top = form_top($data); 
	$end = form_end($data);
	if (!is_array($boxes)) {
		$boxes = ll_boxes($vend,($showkids=true));
	}
	if ($permcheck['boxes']) $div = "&nbsp;-&nbsp;";
	else $div = ' &nbsp;&nbsp; ';
	$url = "<a href=\"".$data['app']."?box=";
	$numboxes = count($boxes);
	if ($numboxes <> 1) $s = 'es';
	$html = <<<HTML
$numboxes box$s
$table
<tr><th><nobr>Box / Paid to</nobr></th><th>Tools</th></tr>
HTML;
	foreach ($boxes as $row) {
		$box = $row['box'];
		$paidto = preg_match('#(0000-00-00|^\s*$)#', $row['paidto']) ? $row['status'] : $row['paidto'];
		$instr = <<<HTML
<a href="index.php?box=$box&seccode={$row['seccode']}&llphone={$row['llphone']}"
   target=_blank>instructions</a>
HTML;
		if ($permcheck['boxes']) {
			$add = "$url$box&form=add\">add time</a>";
			$sub = "$url$box&form=sub\">subtract time</a>";
			$del = "$url$box&form=del\">delete</a>";
		}
		# removed to avoid potential privacy complaints 
		# and to allow us to put vm and web on different servers more easily
		$msg = "$url$box&listen=1\">messages</a>";
		$shsc = showcodelink($box,$row['seccode']);
		$chsc = "$shsc / $url$box&form=chsc\">change security code</a>";
		$edit = "$url$box&form=edit\">edit</a>";
		$v = "<input type=hidden name=vendor value=\"".$row['vendor']."\">";

		if (strlen($row['name']) > 30) $namebr = '<br>';
		else $namebr = '';
		if (strlen($row['notes']) > 20) $notesbr = '<br>';
		else $notesbr = '';
		if (strlen($row['login']) > 32 and $notestbr == '') $loginbr = '<br>';
		else $loginbr = '';

		$html .= <<<HTML
<tr valign=top>
<td><nobr><b>$box</b> &nbsp;&nbsp; $paidto $v</nobr></td>
<td>
<b>vendor:</b> {$row['vid']} &nbsp;
<b>name:</b> {$row['name']} &nbsp; 
$namebr
<b>email:</b> {$row['email']} &nbsp; 
$notesbr
<b>notes:</b> {$row['notes']} &nbsp;
$notesbr$loginbr
<b>last edit:</b> {$row['login']}
</td>
</tr>
<tr bgcolor=lightgray>
<td><a href="admin.php?form=Call+Activity&box=$box">activity</a> / $edit</td>
<td>
<nobr>
$add $div $sub $div $del $div $chsc $div $instr
</nobr>
</td>
</tr>

HTML;
	}
	$html .= <<<HTML
</table>
$end
HTML;
	return $html;
}

function purchase_time_form($data) {
	global $table, $ldata, $min_purchase;

	$top = form_top($data); 
	$end = form_end($data);
	if (credit_left($ldata['vid']) > 0) {
		$rate = sprintf('$%.2f',$ldata['rate']);
		return <<<HTML
$top
<blockquote>
<i>You can use any time you purchase to either create new voice mailboxes or to extend existing boxes.</i>
</blockquote>
Months: &nbsp;&nbsp; <input size=10 name=months value=$min_purchase> &nbsp;&nbsp;
Rate: &nbsp;&nbsp; $rate &nbsp;&nbsp;
<input type=submit name=action value="Purchase time" class=action>
$end
HTML;
	} else {
		return <<<HTML
$top
<blockquote>
<em>You do not have sufficient credit to purchase more voicemail.</em>
$end
HTML;
	}
}

function confirm_purchase_form($data) {
	global $_REQUEST;
	global $min_months;
	global $vend;
	global $table;
	global $ldata;
	$top = form_top($data); 
	$end = form_end($data);
	$months = $_REQUEST['months'];
	if ($months < $min_months) die("Sorry, but the minimum order is $min_months months!");
	$vend = ll_vendor($ldata['vid']);
	if (credit_left($vend['vid']) < $min_months)
		die("You have exceeded your credit limit of {$vend['credit_limit']} months");
	if (!isset($vend['rate']) or $vend['rate'] === '') 
		die("There is a problem with your account. Please contact us.");

	$st = get_salestax(date('Y-m-d'));
	$total = sprintf('%.2f',$months * $vend['rate']);
	if ($vend['gstexempt'])
		$total = sprintf('%.2f',$total/(1+$st['rate']));

	$trans = ll_generate_trans($vend,'invoices');
	return <<<HTML
$top
$table
<input type=hidden name=trans value="$trans">
<tr><td>Months:</td><td align=right>$months <input type=hidden name=months value="$months"></td></tr>
<tr><td><nobr>Total including {$st['name']}:</nobr></td><td align=right>\$$total</td></tr>
<tr><td>&nbsp;</td><td align=right><input type=submit name=action value="Buy voicemail now"></td></tr>
</table>
$end
HTML;
}

function purchase_time($data) {
	global $_REQUEST;
	$vend = ll_vendor($data['vid']);
	$trans = ll_valid_trans($_REQUEST['trans']);
	ll_delete_trans($vend,$trans);
	$invoice = ll_generate_invoice($vend,$_REQUEST['months'],$trans);
	$vend = ll_vendor($data['vid'],true);
	$top = form_top($data); 
	$end = form_end($data);
	return <<<HTML
$top
Thank you for your purchase. <p>
Click on the link below and print the invoice. <br>
<b>Net payment due in 30 days</b><p>
Invoice: <a href=/lifeline/admin.php?action=invoice&invoice=$invoice target=_blank>$invoice</a>
$end
HTML;
}

function list_invoices($data,$showall=false) {
	global $ldata;
	if ($data['vid'] != $ldata['vid'] and $data['parent'] != $ldata['vid']) 
		die("Error: you are trying to view someone else's invoices.");
	$table = table_header(3,0,0,800);
	$top = form_top($data); 
	$end = form_end($data);
	$vend = ll_vendor($data['vid']);
	$invoices = ll_invoices($showall,$vend);
	$owing = sprintf('$%.2f',ll_get_owing($data));
	$invoiced = sprintf('$%.2f',ll_get_invoiced($data));
	$vendor = $vend['vendor'];
	$html = <<<HTML
$top
<h3>Invoices for $vendor ($owing owing)</h3>
$table
<tr><th>invoice</th><th>vendor</th><th>created</th><th>tax</th><th>total</th><th>paid</th></tr>
HTML;
        foreach ($invoices as $invoice) {
                if (preg_match('#^0000#',$invoice['paidon'])) $invoice['paidon'] = '';
                $html .= "<tr valign=top>\n";
		$in = $invoice['invoice'];
		# if you are logged in as the parent then you can edit the invoice
		if ($invoice['parent'] == $ldata['vid']) 
			$editinv = "<td align=right><a href=\"admin.php?form=Edit invoice&invoice=$in\">edit</a>";
		else $editinv = '';
                foreach (array('invoice','vendor','created','gst','total','paidon') as $field) {
			$value = htmlentities($invoice[$field]);
                        if (is_numeric($field)) continue;
			if ($field === 'invoice') 
				$html .= <<<HTML
<td align=center><a href="/lifeline/admin.php?action=invoice&invoice=$in" target=_blank>$in</a></td>
HTML;
                        else if ($field === 'total' or $field === 'gst') {
				$html .= "<td align=right>".(sprintf('$%.2f',$value))." &nbsp;</td>";
			} else if ($field === 'paidon') {
				$html .= "<td align=right>$value $editinv</td>";
			} else $html .= "<td align=right>$value &nbsp;</td>";
                }
                $html .= "</tr>\n";
		if ($invoice['notes']) {
			$notes = htmlentities($invoice['notes']);
			$html .= "<tr bgcolor=lightgray><td colspan=7 align=center><b>Notes:</b> $notes</td></tr>\n";
		}
        }
	$html .= <<<HTML
</table>
$end
HTML;
	return $html;
}

function edit_invoice($invoice) {
	global $ldata;
	if (!preg_match('#^\d+$#', $invoice)) die("invoice should be a number!");

	if (!preg_match('#invoices|^s$#',$ldata['perms'])) 
		die("you do not have permission to view invoices!");

	$idata = ll_invoice($invoice);
	if ($ldata['vid'] != $idata['vdata']['parent']) 
		die("you do not have permission to edit this invoice!");
	
	$table = table_header(3,0,0,600);
	$top = form_top($ldata,true,false); 
	$end = form_end($ldata);
	$idata['gst'] = sprintf('$%.2f', $idata['gst']);
	$idata['total'] = sprintf('$%.2f', $idata['total']);
	$idata['created'] = preg_replace('# .*#','',$idata['created']);
	$idata['paidon'] = preg_replace('# .*#','',$idata['paidon']);
	if ($idata['paidon'] == '0000-00-00') $idata['paidon'] = '';
	$today = date('Y-m-d');
	$html = <<<HTML
$top
<h3>Invoice 
<a href="admin.php?action=invoice&invoice=$invoice" target=_blank>$invoice</a>
for 
<a href="admin.php?vid={$idata['vid']}&form=Show all invoices">{$idata['vdata']['vendor']}</a></h3>
<input type=hidden name=invoice value="$invoice">
<input type=hidden name=vid value="{$idata['vid']}">
$table
<tr><td>invoice</td><td>{$idata['invoice']}</td></tr>
<tr><td>vendor</td><td>{$idata['vdata']['vendor']}</td></tr>
<tr><td>created</td><td>{$idata['created']}</td></tr>
<tr><td>tax</td><td>{$idata['gst']}</td></tr>
<tr><td>total</td><td>{$idata['total']}</td></tr>
<tr><td>paid on</td>
    <td><input id="paidon" name="paidon" value="{$idata['paidon']}" max=10 size=10> 
	YYYY-MM-DD &nbsp;
	<a class=support href="javascript:void(0);" 
	   onclick="topform.paidon.value='$today'; return false;">set to today</a> &nbsp;
	<a class=support href="javascript:void(0);" 
	   onclick="topform.paidon.value=''; return false;">clear</a>
</td></tr>
<tr><td>notes</td><td><input name="notes" value="{$idata['notes']}" size=40></td></tr>
<tr><td><input type=reset value="reset"></td>
    <td align=right><input type=submit name=form value="Save invoice"></td></tr>
</table>
$end
HTML;
	return $html;
}

function invoice($data,$idata=null) {
	global $net_due;
	global $ldata;
	# only let people with invoice viewing permissions to look at invoices
	if (!preg_match('#invoices|^s$#',$ldata['perms'])) 
		die("You don't have the permissions to view invoices.");
	if (!is_array($idata)) {
		$invoice = $_REQUEST['invoice'];
		if (!preg_match('#^\d+$#',$invoice)) 
			die("Invoice is not a number!");

		# invoice is a combo of invoice, vendor and seller data
		$idata = ll_invoice($invoice);
	}
	# note that this will grab the first numeric digits so that should always be
	# the vendor who should receive the funds - ie the ordering in parent is important
	$sdata = ll_vendor((int) ($idata['vdata']['parent']));
	$seller = $sdata['vendor'];
	$vend = $idata['vdata'];

	$total = sprintf('%.2f',$idata[total]);
	$tax = sprintf('%.2f',$idata['gst']);
	$st = get_salestax($idata['created']);
	$created = preg_replace('# .*#','',$idata['created']);
	if (!isset($idata)) die("Could not find invoice $invoice!");
	$table = table_header(3,0,0,500,'align=center');
	return <<<HTML
<html>
<head>
<title>$seller invoice $invoice</title>
<link rel=stylesheet href=css/base.css type=text/css>
<link rel=stylesheet href=css/admin.css type=text/css>
</head>
<body>
<table width=600 align=center cellpadding=10 cellspacing=0>
<tr><td>
<center>
<h2>$seller Invoice $invoice</h2>
$sdata[address]<br>
Phone: $sdata[phone]<br>
Fax: $sdata[fax]<br>
Email: $sdata[email]<br>
GST Number: $sdata[gst_number]<br>
</center>
<p>
$table
<tr><td><b>Invoice Date:</b></td><td>$created&nbsp;</td></tr>
<tr><td><b>To:</b></td><td>$vend[vendor]&nbsp;</td></tr>
<tr><td><b>Contact:</b></td><td>$vend[contact]&nbsp;</td></tr>
<tr><td><b>Purchased by:</b></td><td>$idata[login]&nbsp;</td></tr>
<tr><td><b>Address:</b></td><td>$vend[address]&nbsp;</td></tr>
<tr><td><b>Phone:</b></td><td>$vend[phone]&nbsp;</td></tr>
<tr><td><b>Email:</b></td><td>$vend[email]&nbsp;</td></tr>
</table>
<p>
Please remit <b>\$$total</b> (including \$$tax {$st['name']}) for $idata[months] months voicemail.
<p>
Invoice payable $net_due days from invoice date.
<p>
<p>
Thank you!
<p>
$seller
<p>
<p>
<center><i>Save and print this invoice for your records.</i></center>
</td></tr>
</table>
</body>
</html>
HTML;
}

function view_transaction($ldata,$trans) {
	$tdata = ll_load_from_table('transactions','trans',$trans,$false);
	if (!ll_has_access($ldata,$tdata)) die("you do not have access to this transaction!");

	if ($tdata['box']) {
		$boxes[] = ll_box($tdata['box']);
		return show_boxes($ldata,$boxes);

	} else if ($tdata['table_name'] == 'boxes') {
		$boxes = ll_load_from_table('boxes','trans',$trans);
		return show_boxes($ldata,$boxes);

	} else if ($tdata['table_name'] == 'invoices') {
		$idata = ll_load_from_table('invoices','trans',$trans,false);
		return invoice(null,$idata);
	}
	die("don't know how to find $trans!");
}
