<?php
require_once("$lib/asterisk.php");
require_once("php/lifeline-schema.php");
$back = "<a href=/lifeline/admin.php>Back to admin</a>";
if ($permcheck['logins'])
	$manage = "<a href=\"admin.php?action=Manage account and users\">Manage account and users</a>";
$table = table_header();

function table_header($cp=5,$cs=0,$b=0,$w=300,$style='') {
	return "<table cellpadding=$cp cellspacing=$cs border=$b width=$w $style>";
}

function form_top($data,$show_goback=true,$show_status=true) {
	global $back, $manage;
	global $vend;
	global $form;
	$login = $data['login'];
	$vendor = $data['vendor'];
	$app = $data['app'];
	$time = date('r',$data['time']);
	$vend = ll_vendor($data['vid']);
	if ($show_status) $status = vend_status_str($vend);
	$goback = $show_goback ? "$manage - $back" : '';
	$logout = "<a href=\"admin.php?action=logout\">Log out</a>";
	return <<<HTML
<html>
<head>
<title>Voicemail Admin: $form</title>
<link rel=stylesheet type=text/css href=/lifeline/css/admin.css>
</head>
<body bgcolor=lightyellow>
<center>
<h4>Vendor: $vendor <span style="font-weight: normal;">$goback &nbsp;&nbsp; $logout</span></h4>
<h3>$form</h3>
$status<p>
<form action=/lifeline/admin.php name="topform" id="topform" method=get>
HTML;
}

function form_end($data) {
	global $adminfo;
	$uptime = get_uptime();
	return <<<HTML
</form>
<p>
<span class="support">
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
	global $ldata;
	global $min_purchase;
	$top = form_top($data,false); 
	$end = form_end($data);
	if (preg_match('#invoices#',$data['perms'])) {
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
<input type=submit name=form value="Show all invoices"> &nbsp;&nbsp;
<input type=submit name=form value="Show unpaid invoices">
</nobr>
<p>
HTML;
	}
	if (preg_match('#logins#',$data['perms'])) 
		$users = '<input type=submit name=action value="Manage account and users"> <p>';
	return <<<HTML
$top
<input type=submit name=form value="Create a new voicemail box"> <p>
<input type=submit name=form value="Add time to an existing box"> <p>
<input type=submit name=form value="View your voicemail boxes"> <p>
$users
$purchase
$end
HTML;
}

function vend_status_str($vend) {
	global $min_purchase;
	if ($vend['months'] == 1) {
		$months = "1 month of";
	} else if ($vend['months'] == 0) {
		$months = "no";
	} else {
		$months = $vend['months']." months of";
	}
	$status = "{$vend['vendor']} has $months voicemail available.<br>";
	return $status;
}
 
function create_new_box_form($data) {
	global $vend;
	$top = form_top($data); 
	$end = form_end($data);
	$trans = ll_generate_trans($vend);
	$personal = mk_personal_input();
	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
New box valid for &nbsp; <input size=3 name=months value=1> &nbsp; month(s). &nbsp;&nbsp; 
<input type=checkbox name=activate value=1> Activate now &nbsp;&nbsp;
$personal
<input type=submit name=action value="Create box" class=action>
$end
HTML;
}

function create_new_box($data) {
	global $_REQUEST;
	global $table;
	$vend = ll_vendor($data['vid']);
	ll_delete_trans($vend,$_REQUEST['trans']);

	$months = $_REQUEST['months'];
	if (!preg_match('#^\d\d?$#',$months)) die("create_new_box: invalid number of months");

	list($min_box,$max_box) = get_box_range(); # pick a random box range 
	list ($box,$seccode,$paidto) = ll_new_box($vend,$months,$min_box,$max_box,$_COOKIE['activate']);
	ll_update_personal($vend,$box,$_REQUEST['personal']);

	$vend = ll_vendor($data['vid'],true);

	$top = form_top($data); 
	$end = form_end($data);

	$instr = file_get_contents("index.php");
	$bdata = ll_box($box);
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td>$box</td></tr>
<tr><td><b>Security code:</b></td><td>$seccode</td></tr>
<tr><td><b>Paid to:</b></td><td>$paidto</td></tr>
<tr><td><b>Status</b></td><td>{$bdata['status']}</td></tr>
<tr><td><b>Name:</b></td><td>{$bdata['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$bdata['email']}</td></tr>
<tr><td><b>Notes:</b></td><td>{$bdata['notes']}</td></tr>
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
	$box = $_REQUEST['box'];
	if (preg_match('#^\d+$#',$box)) $bdata = ll_box($box);
	if (preg_match('#^\d#',$bdata['paidto'])) {
		$paidto = preg_replace('# .*#','',$bdata['paidto']);
		$status = "(Paid to $paidto, status {$bdata['status']})";
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
			$months = ll_months_left($bdata['paidto']);
			if ($months <= 0) 
				return "$top<h3>Box $box has less than one month left on its subscription.</h3>$end";
		} else $months = 1;
		$month_input = " Months: <input size=3 name=months value=$months> &nbsp;&nbsp;"; 
		$trans = ll_generate_trans($vend);
	}
	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
Box: <input $is_hidden name=box size=7 value="$box"><b>$box</b> $status &nbsp;&nbsp;
$seccode_input
$month_input
$edit_input
<input type=submit name=action value="$action" class=action>
$end
HTML;
}

function mk_personal_input($bdata=array()) {
	global $table;
	return <<<HTML
<p>
$table
<tr><td>Name:</td><td><input size=32 name="personal[name]" value="{$bdata['name']}"></td></tr>
<tr><td>Email:</td><td><input size=40 name="personal[email]" value="{$bdata['email']}"></td></tr>
<tr><td>Notes:</td><td><input size=64 name="personal[notes]" value="{$bdata['notes']}"></td></tr>
</table>
<br>

HTML;
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
	$vend = ll_vendor($data['vid']);
	$box = $_REQUEST['box'];
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

function update_box_time($data,$months='') {
	global $_REQUEST;
	global $table;
	$vend = ll_vendor($data['vid']);
	ll_delete_trans($vend,$_REQUEST['trans']);
	if ($months === '') $months = $_REQUEST['months'];
	$box = $_REQUEST['box'];
	$paidto = ll_add_time($vend,$box,$months);
	$vend = ll_vendor($data['vid'],true);
	$top = form_top($data); 
	$end = form_end($data);
	if ($months > 0) $instr = file_get_contents("index.php");
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td>$box</td></tr>
<tr><td><b>Paid to:</b></td><td>$paidto</td></tr>
</table>
$instr
$end
HTML;
}

function update_personal($data) {
	global $table, $vend;
	$top = form_top($data);
	$end = form_end($data);
	$box = $_REQUEST['box'];
	if (!preg_match('#^\d+$#',$box)) 
		die("update_personal: box should be a number not $box!");
	ll_update_personal($vend,$box,$_REQUEST['personal']);
	$bdata = ll_box($box);
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td>$box</td></tr>
<tr><td><b>Name:</b></td><td>{$bdata['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$bdata['email']}</td></tr>
<tr><td><b>Notes:</b></td><td>{$bdata['notes']}</td></tr>
</table>
$end

HTML;
}

function find_boxes_form($data,$boxes=null) {
	global $table;
	$top = form_top($data); 
	$end = form_end($data);
	if (!is_array($boxes)) $boxes = ll_boxes($data,'not_deleted','order by box');
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

function view_boxes_form($data,$boxes=null) {
	global $table;
	global $vend;
	$top = form_top($data); 
	$end = form_end($data);
	if (!is_array($boxes)) {
		$boxes = ll_boxes($vend);
	}
	$div = "&nbsp;-&nbsp;";
	$url = "<a href=\"".$data['app']."?box=";
	$search = htmlentities($_REQUEST['search']);
	$numboxes = count($boxes);
	if ($numboxes <> 1) $s = 'es';
	$html = <<<HTML
<form action=admin.php method=get>
<input name="search" value="$search"> 
<input type=hidden name="vid" value="{$data['vid']}"> 
<input type=submit name=form value="Search">
<br>
<a href="admin.php?form=View your voicemail boxes&vid={$data['vid']}">Show all</a>
</form>
$numboxes box$s
$table
<tr><th><nobr>Box / Paid to</nobr></th><th>Tools</th></tr>
HTML;
	foreach ($boxes as $row) {
		$box = $row['box'];
		$paidto = $row['paidto'] == '0000-00-00' ? $row['status'] : $row['paidto'];
		$add = "$url$box&form=add\">add time</a>";
		$sub = "$url$box&form=sub\">subtract time</a>";
		$del = "$url$box&form=del\">delete</a>";
		# removed to avoid potential privacy complaints 
		# and to allow us to put vm and web on different servers more easily
		$msg = "$url$box&listen=1\">messages</a>";
		$chsc = "$url$box&form=chsc\">change security code</a>";
		$edit = "$url$box&form=edit\">edit</a>";
		$v = "<input type=hidden name=vendor value=\"".$row['vendor']."\">";
		$html .= <<<HTML
<tr><td><nobr>$box &nbsp;&nbsp; $paidto $v</nobr></td>
<td>
<nobr>
<b>name:</b> {$row['name']} &nbsp; <b>email:</b> {$row['email']} &nbsp; <b>notes:</b> {$row['notes']} 
</nobr>
</td>
</tr>
<tr bgcolor=lightgray>
<td>$edit</td>
<td>
<nobr>
$add $div $sub $div $del $div $chsc
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
		return <<<HTML
$top
<blockquote>
<i>You can use any time you purchase to either create new voice mailboxes or to extend existing boxes.</i>
</blockquote>
Months: &nbsp;&nbsp; <input size=10 name=months value=$min_purchase> &nbsp;&nbsp;
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

	$trans = ll_generate_trans($vend);
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
	ll_delete_trans($vend,$_REQUEST['trans']);
	$invoice = ll_generate_invoice($vend,$_REQUEST['months']);
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

function invoice($data) {
	global $net_due;
	global $ldata;
	# only let people with invoice viewing permissions to look at invoices
	if (!preg_match('#invoices|^s$#',$ldata['perms'])) 
		die("You don't have the permissions to view invoices.");
	$invoice = $_REQUEST['invoice'];
	if (!preg_match('#^\d+$#',$invoice)) 
		die("Invoice is not a number!");

	# invoice is a combo of invoice, vendor and seller data
	$idata = ll_invoice($invoice);
	$sdata = ll_vendor($idata['vdata']['parent']);
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

