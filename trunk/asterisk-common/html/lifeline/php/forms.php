<?php
require_once("$lib/asterisk.php");
require_once("php/lifeline-schema.php");
# this doesn't work if you go from admin.php to clients.php links will be for the wrong page
# $script = $_SERVER['PHP_SELF'];
$back = "<a href='$script?'>Back to admin</a>";
$table = table_header();

function table_header($cp=5,$cs=0,$b=0,$w=850,$style='') {
	return "<table cellpadding=$cp cellspacing=$cs border=$b width=$w $style>";
}

function is_admin_php() {
	return ($_SERVER['PHP_SELF'] == '/lifeline/admin.php');
}

function is_clients_php() {
	return ($_SERVER['PHP_SELF'] == '/lifeline/clients.php');
}

function head() {
	global $form,$ldata,$sitecolor;
	$title = empty($form) ? "main page" : $form;
	if ($ldata['retail_prices'] and $form == 'Create a new voicemail box') {
		$pairs = explode(';',$ldata['retail_prices']);
		foreach ($pairs as $pair) {
			list($months,$amount) = explode('=',$pair);
			$retail_prices[$months] = $amount;
			$prices .= "retail_prices[$months] = '$amount';\n";
		}
		$retail_script = <<<JS
<script type=text/javascript>
var retail_prices = new Array();
$prices
function get_retail_price(months) {
	if (months <= 0) {
		alert('Invalid number of months! Should be 1 or more.');
		return '0.00';
	}
	if (retail_prices[months]) return retail_prices[months];
	return '0.00';
}
</script>
JS;
	}
	return <<<HTML
<html>
<head>
<title>Voicemail Admin: $title</title>

<!-- from http://jqueryui.com/datepicker/ -->
<link rel="stylesheet" href="code.jquery.com/ui/1.11.2/themes/smoothness/jquery-ui.css">
<script src="code.jquery.com/jquery-1.10.2.js"></script>
<script src="code.jquery.com/ui/1.11.2/jquery-ui.js"></script>
<script> $(function() { $( "#datepicker" ).datepicker({dateFormat:"yy-mm-dd"}); }); </script>

<link rel=stylesheet type=text/css href=/lifeline/css/admin.css>
$retail_script
</head>
<body class="site" bgcolor="$sitecolor">
HTML;
}

function form_wrap($html,$method="get") {
	global $script;
	return <<<HTML
<form action="$script" method="$method">
$html
</form>
HTML;
}

function client_search($data) {
	$search = htmlentities($_REQUEST['search']);
	return <<<HTML
<input type="text" name="search" value="$search" size="40">
<input type="submit" name="action" value="Search Clients">
<input type=hidden name="vid" value="{$data['vid']}">
<br>
<i>to search for a PO BOX type</i> <b>pobox</b>
HTML;
}

function form_top($data,$show_goback=true,$show_status=true,$method='get',$formjs='') {
	global $back, $manage, $script;
	global $vend, $ldata;
	global $form;
	$login = $data['login'];
	$vendor = $data['vendor'];
	$time = date('r',$data['time']);
	$vend = ll_vendor($data['vid']);
	if ($show_status) $status = vend_status_str($vend);
	$goback = $show_goback ? "&nbsp;&nbsp; $back" : '';
	if ($ldata) {
		$logout = <<<HTML
<nobr>
<a href="$script?action=logout">Log out</a>
HTML;
		if (preg_match('#^\d+$#', $ldata['orig_vid']) and $ldata['orig_vid'] == $ldata['initial_vid']) {
			$logout .= <<<HTML
&nbsp; 
/ &nbsp; <a href="$script?switch_vendor={$ldata['orig_vid']}">
Return to {$ldata['orig_vendor']}</a>
HTML;
		}
		$logout .= "</nobr>\n";
		if ($ldata['vid'] == POBOXVID and !is_clients_php()) {
			$logout .= <<<HTML
&nbsp;
<a href="clients.php" target="_blank">Clients page (manage PO Boxes)</a>
HTML;
		}
	}
	$head = head();
	if (is_admin_php()) $search_form = search_form($data);
	else if (is_clients_php()) $search_form = form_wrap(client_search($data));
	if (!is_clients_php()) $docs = "<a href=\"docs\">Documentation</a>";
	return <<<HTML
$head
<center>
<h4>Vendor: $vendor <span style="font-weight: normal;">$goback &nbsp;&nbsp; $logout &nbsp;&nbsp; $docs</span></h4>
$status<p>
$search_form
<h3>$form</h3>
<form action="$script" name="topform" id="topform" method="$method" $formjs>
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

# status can be 'deleted' or 'not_deleted' 
# only meaningful from clients form
function poboxes($status='all', $box=null, $field='pobox') {
	if (!is_clients_php()) return "";
	$boxes = ll_poboxes($status);
	$html = "<select name='$field'><option value=''></option>\n";
	foreach ($boxes as $bdata) {
		if ($box == $bdata['box']) {
			$selected = "selected";
		} else {
			$selected = "";
		}
		if ($bdata['paidto'] < date('Y-m-d')) $paidto = 'box available';
		else $paidto = "paid to {$bdata['paidto']}";
		$name = substr($bdata['name'],0,20);
		if (strlen($name) < strlen($bdata['name'])) $name .= "...";
		$html .= "<option value='{$bdata['box']}' $selected title='{$bdata['paidto']} {$bdata['name']}'>{$bdata['box']}".
			" - $paidto ($name)</option>\n";
	}
	$html .= "</select>\n";
	return $html;
}

function poboxform() {
	$poboxes = poboxes('all');
	return <<<HTML
PO Boxes:
<br>
$poboxes
<input type=submit name=form value="Edit PO Box">
<p>
<input type=submit name=form value="Print PO Box Reminders">
HTML;
}

function main_clients_form($data) {
	global $ldata, $permcheck, $overdueblock, $vdata;

	$top = form_top($data,false); 
	$end = form_end($data);
	$poboxform = poboxform();
	if (!$overdueblock and $ldata['months'] >= 0) {
		$addtime_buttons = <<<HTML
<p>
Voicemail:
<br>
<input type=submit name=form value="Create a new voicemail box"> <p>
<input type=submit name=form value="Add time to an existing box"> <p>
HTML;
	}
	return <<<HTML
$top
<p>
$addtime_buttons
<br>
$poboxform
$end
HTML;
}

function pobox_reminders_form($data) {

	if (preg_match('#^\d+$#', $_REQUEST['pobox'])) {
		$poboxes[] = ll_pobox($_REQUEST['pobox']);
	} else {
		if (preg_match('#^\d\d\d\d-\d\d-\d\d$#', $_REQUEST['date'])) 
			$date = $_REQUEST['date'];
		else $date = date('Y-m-d');
		$poboxes = ll_poboxes('recent_first',$date);
	}
	if (preg_match('#^\d+$#', $_REQUEST['break'])) $break = $_REQUEST['break'];
	else $break = 5;

	$html = <<<HTML
<html>
<head>
<title>PO Box Reminders for $date</title>
<style type="text/css">
.pobox-reminder pre {
	font-family: helvetica, arial, sans-serif;
	font-size: 12px;
}
.pobox-reminder {
	width: 600px; 
	height: 110px;
	padding-top: 20px; 
	padding-bottom: 20px;
}
.pobox-divider {
	width: 600px; 
}
.pobox-pagebreak {
	page-break-after: always;
}
</style>
</head>
<body>
HTML;
	foreach ($poboxes as $pobox) {
		if (++$pbc % $break == 0) {
			$pbreak = "<div class='pobox-pagebreak'></div>\n";
		} else {
			$pbreak = "";
		}
		$html .= <<<HTML
<div class="pobox-reminder">
<pre>
Dear {$pobox['name']},

This is just a reminder that PO Box {$pobox['box']} is paid to {$pobox['paidto']}.
If you would like to renew your subscription please see us at the front desk.

Thanks!
</pre>
</div>
<div class="pobox-divider">
<hr>
</div>
$pbreak
HTML;
	}
	$html .= "</body></html>";
	return $html;
}

function main_form($data) {
	global $ldata, $permcheck, $min_purchase, $overdue, $overdueblock, $vdata;

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
						(abs($credit))." month$s";
			} else {
				$remaining = "You can purchase $credit month$s more voicemail.";
			}
		}
		if ($vend['acctype'] == 'purchase') {
			if ($credit >= $min_purchase and count($overdue) == 0) {
				$purchase = <<<HTML
<input type=submit name=form value="Purchase time">
<br>
$limit
$remaining
<br>
HTML;
			} else {
				if (count($overdue)) $payinvoices = overdueinvoices($overdue);
				else $payinvoices = 
					"<em>Please pay outstanding invoices before purchasing voice mail.</em>";
				$purchase = <<<HTML
$payinvoices
<br>
$limit
$remaining
HTML;
			}
		}
		if (($outstanding = list_invoices($vdata,false,false)) === false) {
			$purchase .= <<<HTML
<input type=submit name=form value="Show all invoices"> 
<p>
HTML;
		} else {
			$purchase .= $outstanding;
		}
	}
	if ($vdata['vid'] == ROOTVID or ($lines = ll_free_phones($vdata['vid']))) {
		$lastweek = date('Y-m-d',time()-(7*86400));
		$now = date('Y-m-d');
# they stopped using it and never returned equipment
$now = '2012-08-17';
$lastweek = '2012-08-10';
		$free_phone_form = <<<HTML
<table>
<tr>
<td>
From: <input name=from value="$lastweek" size=10>
</td>
<td>
To: <input name=to value="$now" size=10>
</td>
<td>
<input type=submit name=action value="Get free phone logs"> 
<input type=hidden name=vid value="{$vdata['vid']}">
</td>
</tr>
</table>
<p>
HTML;
	}
	if (!$overdueblock and $ldata['months'] > 0) {
		$addtime_buttons = <<<HTML
<input type=submit name=form value="Create a new voicemail box"> <p>
<input type=submit name=form value="Add time to an existing box"> <p>
HTML;
	}

	if (preg_match('#logins#',$data['perms'])) 
		$users = '<input type=submit name=action value="Manage account and users"><p>';
		if ($data['vid'] == $data['parent']) {
			$users .= '<input type=submit name=action value="Recent logins"><p>';
		}
	return <<<HTML
$top
$free_phone_form
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
	global $min_purchase, $permcheck, $overdue, $overdueblock, $script;
	if ($overdueblock) {
		return "<b>You must pay overdue invoices before you can create new voice mail boxes.</b>";
	}

	if ($vend['months'] == 0 and $vend['acctype'] == 'login') 
		return;

	if ($vend['months'] == 1) {
		$months = "1 month";
	} else if ($vend['months'] == 0) {
		$months = "no";
		if ($permcheck['invoices'] and $vend['acctype'] == 'purchase') {
			$purchaselink = "<a href=\"$script?form=Purchase time\">Purchase time.</a>";
		}
		
	} else {
		$months = $vend['months']." months";
	}
	if (count($overdue)) {
		$purchaselink = "Some invoices overdue.";
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
	} else if (is_clients_php()) {
		$months = 1;
		$boxeswidget = <<<HTML
Number of boxes to create: &nbsp; 1 <input type=hidden name=boxes value=1>  &nbsp;&nbsp; 
HTML;
	} else {
		$months = 1;
		$boxeswidget = <<<HTML
<span id="numboxes" style="display: inline">
Number of boxes to create &nbsp; 
<input size=3 name=boxes value=1> (maximum $max) 
<a href="javascript: void(0);" onclick="
   boxwidget = getElementById('specificbox');
   numwidget = getElementById('numboxes');
   entrydesc = getElementById('entrydesc');
   if (boxwidget.style.display == 'inline') {
	boxwidget.style.display = 'none';
	numwidget.style.display = 'inline';
	entrydesc.innerHTML = 'click * to enter a specific box number';
   } else {
	boxwidget.style.display = 'inline';
	numwidget.style.display = 'none';
	entrydesc.innerHTML = 'click * to enter number of boxes';
   }
" 
   title="click * to enter a specific box number">*</a>
</span>
<span id="specificbox" style="display: none">
Reactivate inactive box &nbsp;
<input name="box" size="4">
<a href="javascript: void(0);" onclick="
   boxwidget = getElementById('specificbox');
   numwidget = getElementById('numboxes');
   entrydesc = getElementById('entrydesc');
   if (numwidget.style.display == 'none') {
	boxwidget.style.display = 'none';
	numwidget.style.display = 'inline';
	entrydesc.innerHTML = 'click * to enter a specific box number';
   } else {
	boxwidget.style.display = 'inline';
	numwidget.style.display = 'none';
	entrydesc.innerHTML = 'click * to enter number of boxes';
   }
" 
   title="click * to enter number of boxes">*</a>
</span>
</span> &nbsp;&nbsp; 
HTML;
	}
	$top = form_top($data,true,true,'post',$formjs); 
	$end = form_end($data);
	$trans = ll_generate_trans($vend,'boxes');
	$personal = mk_personal_input(null,$vend);
	$payment_form = payment_form($box,defpayment($vend,$months));
	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
$boxeswidget
Valid for &nbsp; 
<input size=3 name=months value="$months"
 onchange="getElementById('payment_amount').value=get_retail_price(this.value);"
> &nbsp; months (up to $maxmonths months). &nbsp;&nbsp; 
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
function payment_form($box,$defpayment=0.0) {
	global $ldata,$vend;
	if (!preg_match('#^\d+$#',$box)) $box = "";

	$payment = $_REQUEST['payment'];
	if (!is_array($payment)) $payment = array();

	$now = $payment['paidon'];
	if (!preg_match('#^\d{4}-\d{2}-\d{2}#',$now)) $now = date('Y-m-d H:i:s');

	$amount = $payment['amount'];
	if (!is_numeric($amount)) $amount = sprintf('%.2f', $defpayment);

	$notes = htmlentities($payment['notes']);

	if ($box) $forbox = "for $box";

	return <<<HTML
<h4>Payment info $forbox (leave as is if not needed)</h4>
<table cellpadding=5 cellspacing=0 border=0>
<tr><td>Paid On</td><td><input size=20 name="payment[paidon]" value="$now"></td></tr>
<tr><td>Amount Paid</td>
<td>
<input size=10 id=payment_amount name="payment[amount]" value="$amount"></td></tr>
<tr><td>Notes</td><td><input size=40 name="payment[notes]" value="$notes"></td></tr>
</table>

HTML;
}

function create_new_box($data,$box=null) {
	global $ldata, $phone;
	$vend = ll_vendor($data['vid']);
	$trans = ll_valid_trans($_REQUEST['trans']);
	ll_delete_trans($vend,$trans,'new');

	if (isset($box)) {
		if (!preg_match('#^\d+$#', $box)) die("invalid box entered!");

	} else if (preg_match('#^\d+$#', $_REQUEST['box'])) {
		$box = $_REQUEST['box'];

	} else {
		$boxes = $_REQUEST['boxes'];
		if (!preg_match('#^\d\d?$#',$boxes) or $boxes <= 0) die("create_new_box: invalid number of boxes");
		if ($boxes > MAXBOXES) die("please select a smaller number of boxes than ".MAXBOXES);
	}

	$months = $_REQUEST['months'];
	if (!preg_match('#^\d\d?$#',$months) or $months <= 0) die("create_new_box: invalid number of months");
	if ($months > MAXMONTHS) die("please select a smaller number of months than ".MAXMONTHS);

	$totalmonths = $months * $boxes;
	if ($vend['months'] < $totalmonths) die("you only have {$vend['months']} months available!");
	$netmonths = $vend['months'];
	$vid = $ldata['vid'];

	$llphone = $_REQUEST['personal']['llphone'];
	if (empty($llphone)) $llphone = $phone;

	if (!isset($box)) {
		list($min_box,$max_box) = get_box_range(); # pick a random box range 
	} else { 
		$max_box = $min_box = $box;
	}
	if (isset($box) or $boxes == 1) {
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

function list_logins($data,$limit=LOGINOUTPUTLIMIT) {
	$top = form_top($data); 
	$end = form_end($data);
	if (!$limit) $limit = LOGINOUTPUTLIMIT;
	$logins = ll_get_logins($limit);
	$html = <<<HTML
$top
<h4>Last $limit logins</h4>
<table cellpadding=5 cellspacing=0 border=1>
HTML;
	foreach ($logins as $login) {
		$html .= "<tr><td>".implode("&nbsp;</td><td>", array_unique(array_values($login)))."</td></tr>\n";
	}
	$html .= <<<HTML
</table>
$end;
HTML;
	return $html;
}

function show_boxes($data,$boxlist) {
	global $lib, $script;
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
		if (is_clients_php()) {
			$searchth = <<<HTML
<th><a href="$script?form=Search Clients&search=$box">$box</a></th>
HTML;
		} else {
			$searchth = <<<HTML
<th><a href="$script?form=Search Boxes&search=$box">$box</a></th>
HTML;
		}

		$html .= <<<HTML
<tr>
$searchth
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
	global $script;
	$box = htmlentities($box);
	$seccode = htmlentities($seccode);
	$showcode = "$script?box=$box&seccode=$seccode&form=showcode";
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
	$poboxtr = poboxtr($bdata);
	return <<<HTML
$top
$table
<tr><td><b>Extension:</b></td><td><a href="?form=Search+Boxes&search=$box">{$bdata['llphone']} Ext $box</a></td></tr>
$poboxtr
<tr><td><b>Status:</b></td><td>{$bdata['status']} $paidto</td></tr>
<tr><td><b>Name:</b></td><td>{$personal['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$personal['email']}</td></tr>
<tr><td><b>Amount:</b></td><td>{$amount}</td></tr>
<tr valign=top><td><b>Notes:</b></td><td>{$personal['notes']}</td></tr>
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

# for displaying a pobox you alread have
function poboxtr($bdata) {
	if ($bdata['cid']) {
		$poboxes = ll_clients_pobox($bdata['cid']);
		if (is_array($poboxes) and count($poboxes) > 0) {
			$poboxrow = "<tr><td><b>PO Box:</b></td><td>";
			$poboxrow .= $poboxes['box']." &nbsp;";
			$poboxrow .= "</td></tr>";
		}
	}
	return $poboxrow;
}
		
# for selecting a pobox 
function poboxrow($bdata) {
	if (!is_clients_php()) return "";
	if (isset($bdata['box'])) {
		$pobox = ll_clients_pobox($bdata['cid']);
		$poboxes = poboxes('all',$pobox['box'],'pobox');
	} else {
		$poboxes = poboxes('deleted_first',null,'pobox');
	}
	return <<<HTML
<tr><td>PO Box:</td><td>$poboxes</td></tr>
HTML;
}

function update_pobox_form($data,$action) {
	return update_box_form($data,$action,'poboxes');
}

function update_box_form($data,$action="Add time to box",$source='boxes') {
	global $_REQUEST;
	global $vend;
	global $form;
	global $table;
	$form = $action;
	$submittype = 'action';
	if ($source == 'boxes') {
		$box = $_REQUEST['box'];
		if (preg_match('#^\d+$#',$box)) $bdata = ll_box($box);

		# set status string
		if (preg_match('#^2\d\d\d#',$bdata['paidto'])) {
			$paidto = preg_replace('# .*#','',$bdata['paidto']);
			$status = "(Paid to $paidto";
			if ($bdata['status']) $status .= ", status {$bdata['status']}";
			$status .= ")";
		} else if ( preg_match('#add (\d+) month#', $bdata['status'], $m) 
				and  $action === 'Update name, email etc.'
		) {
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
<i>Subscription date will be set to $months month$s from this date.<br> 
   Dates must be no later than 2 weeks from today.</i> 
</div>
HTML;
		} else {
			if ($bdata['status']) $status = "(".htmlentities($bdata['status']).")";
		}
		$poboxrow = poboxrow($bdata);
	} else if ($source == 'poboxes') {
		$box = $_REQUEST['pobox'];
		if (preg_match('#^\d+$#',$box)) $bdata = ll_pobox($box);
		$paidto = htmlentities(preg_replace('# .*#','',$bdata['paidto']));
		$status = <<<HTML
&nbsp;&nbsp;
Paid to <input size="10" id="datepicker" name="personal[paidto]" value="$paidto"
	 title="click on the field to show a calendar"> 
 <a href="javascript: void(0);" onclick="$('#datepicker').val(''); return false;">clear</a> &nbsp;
 <a href="?action=Print+PO+Box+Reminders&pobox=$box">print reminder</a>
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
	} else if ($action === 'Update PO Box name, email etc.') {
		$edit_input = mk_personal_input($bdata,null,'poboxes');
	} else if (preg_match('#\btime\b#',$action)) {
		if ($action === 'Remove time from box') {
			$remove = 1;
			$monthsleft = ll_box_months_left($bdata);
			# this is not getting updated correctly and is giving errors
			# canremove only applies when a box has been moved from one vendor to another
			# wcg for example has a group of sub accounts where this logic wouldn't apply
			# if ($bdata['canremove'] < $monthsleft) $monthsleft = $bdata['canremove'];
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

function mk_personal_input($bdata=array(),$vend=null,$source='boxes') {
	global $ldata,$table,$phone,$ll_host;
	if (!is_array($vend) and $bdata['vid']) $vend = ll_vendor($bdata['vid']);
	# if (empty($bdata['notes'])) $bdata['notes'] = $vend['vendor'].' '.$vend['paycode'];
	$myphone = $bdata['llphone'];
	if (empty($myphone)) $myphone = $vend['llphone'];
	if (empty($myphone)) $myphone = $phone;
	$phones = explode(':',$vend['llphone']);
	if ($source == 'boxes') {
		if (count($phones) > 1) {
			# this is mainly for jobwave / triumph	
			$vanpat = '#van|burnaby|surrey|langely|coquitlam|maple\s*ridge|richmond#';
			$tollfreepat = '#^\s*(1\D*|)8\d\d#';
			$phonesel = "<select name=\"personal[llphone]\">";
			$islocal = preg_match($vanpat, $ldata['login']);
			$selectedfound = false;
			foreach ($phones as $ph) {
				$selected = '';
				if (!$selectedfound) {
					if (preg_replace('#\D#','',$ph) == preg_replace('#\D#','',$bdata['llphone'])) {
						$selected = 'selected';
						$selectedfound = true;
					} else if (!$islocal and preg_match($tollfreepat, $ph)) {
						$selected = 'selected';
						$selectedfound = true;
					} else if ($islocal and !preg_match($tollfreepat,$ph)) {
						$selected = 'selected';
						$selectedfound = true;
					}
				}
				$phonesel .= "<option $selected>$ph\n";
			}
			$phonesel .= "</select>\n";
		} else {
			$phlen = strlen($myphone)+1;
			# because of the 877 number don't let people change the phone
			# $phonesel = "<input name=\"personal[llphone]\" value=\"$myphone\" size=\"$phlen\">";
			$phonesel = $myphone;
		}
		$phonerow = <<<HTML
<tr><td><nobr>VM Phone:</nobr></td><td>$phonesel (incoming voicemail number)</td></tr>
HTML;
		$poboxrow = poboxrow($bdata);
	}
	return <<<HTML
<p>
$table
$phonerow
$poboxrow
<tr><td>Name:</td><td><input size=32 name="personal[name]" value="{$bdata['name']}"></td></tr>
<tr valign=top>
    <td>Email:</td>
    <td><input size=40 name="personal[email]" value="{$bdata['email']}"><br>
        <input type="hidden" name="personal[cid]" value="{$bdata['cid']}">
	<i><b>only</b> enter email if client wants email reminders</i></td></tr>
<tr><td>Notes:</td><td><input size=60 name="personal[notes]" value="{$bdata['notes']}"></td></tr>
</table>

HTML;
}

function show_free_phone_logs($data) {
	global $ldata;
	if ($data === false) die('No data found - error');
	$top = form_top($ldata); 
	$end = form_end($ldata);
	$html = <<<HTML
$top
<table border=1 cellpadding=3 cellspacing=0>
<tr>
<th>Call type</th>
<th>Start</th>
<th>Duration (min)</th>
<th>To phone</th>
<th>Free phone id</th>
</tr>

HTML;
	foreach ($data as $line) {
		$phone = preg_replace('#^(\d\d\d)(\d\d\d).*#',"($1) $2-XXXX",$line['phone']);
		$mins = $line['duration'] > 0 ? $line['duration'] : 0;
		$html .= <<<HTML
<tr>
<td>{$line['calltype']} &nbsp;</td>
<td>{$line['start']} &nbsp;</td>
<td align=right>$mins</td>
<td>$phone &nbsp;</td>
<td>{$line['freephone']} &nbsp;</td>
</tr>

HTML;
	}
	$html .= "</table>\n$end";
	return $html;
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
	global $permcheck,$script;
	$html = <<<HTML
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
		if ($p['amount']) $byvendor[$p['vendor']] += $p['amount'];
		$html .= <<<HTML
<tr valign=top>
<td>$item</td>
<td><a href="$script?form=Search Boxes&search={$p['box']}">{$p['box']}</a></td>
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
	if (count($byvendor)) {
		$summary = <<<HTML
<table class="noformat">
<tr><td>Totals:</td><td>
<table cellpadding=5 cellspacing=0 border=0>
<tr>
HTML;
		foreach ($byvendor as $vendor => $total) {
			if ($total == 0) continue;
			$amount = sprintf('$ %.2f', $total);
			$summary .= "<td><b>$vendor</b> $amount</td>\n";
		}
		$summary .= "</tr>\n</table>\n</td></tr></table>\n";
	}
	return $title.$summary."<p>".$html;
}

function callhtml($box,$limit=50) {
	$calls = ll_calls($box,$limit);
	$box = htmlentities($box);
	if (!count($calls)) return "<h4>No activity for box $box</h4>";
	return formatcallhtml("<h4>Recent login attempts and messages for box $box</h4>", $calls);
}

function formatcallhtml($title,$calls) {
	global $script;
	$callhtml = <<<HTML
<table cellpadding=5 cellspacing=0 border=1>
<tr><th>#</th><th>Box</th><th>Vendor Id</th><th>Time</th><th>Action</th><th>Caller ID</th></tr>

HTML;
	foreach ($calls as $call) {
		switch($call['action']) {
		case 'll-flagmsg.pl': 	$calltype = 'message left'; break;
		case 'll-login.pl': 	$calltype = "login: status {$call['status']}"; break;
		case 'll-saveseccode.pl': $calltype = "change security code"; break;
		case 'll-valid.pl': 	$calltype = "box {$call['status']}"; break;
		case 'll-callstart.pl': $calltype = "call start"; break;
		default: 		$calltype = $call['action']." ".$call['status']; 
		}
		# the vid of 0 is reserved so we can detect events with no box # 
		if ($call['vid']) $vendlink = vendlink($call['vid']);
		else $vendlink = '&nbsp;';
		$callerid = htmlentities($call['callerid']);
		$i++;
		$sums[$calltype]++;
		$callstart = date('Y-m-d H:i:s',$call['callstart']);
		$callhtml .= <<<HTML
<tr>
<td>$i</td>
<td><a href="$script?form=Search Boxes&search={$call['box']}">{$call['box']}</a></td>
<td>$vendlink</td>
<td>$callstart &nbsp;</td>
<td>$calltype &nbsp;</td>
<td>$callerid &nbsp;</td>
</tr>
HTML;
	}
	$callhtml .= "</table>\n";
	$summary = "Summary: <table cellpadding=5 cellspacing=0 border=0>\n<tr>\n";
	ksort($sums);
	foreach ($sums as $ctype => $count) {
		$summary .= "<td><b>$ctype</b> $count&nbsp;</td>\n";
	}
	$summary .= "</tr>\n</table>\n";
	return $title.$summary."<p>".$callhtml;
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
	global $ldata;
	$box = $_REQUEST['box'];
	$vend = ll_vendor($data['vid'],true);
	$months = ll_delete_box($vend,$box);
	$s = $months == 1 ? '' : 's';
	# make an audit trail record of the box
	$amount = ll_update_payment(
			$box,
			$data['vid'],
			$ldata['login'],
			-1 * $months,
			array(
				'paidon' => date('Y-m-d H:i:s'),
				'amount' => 0,
				'notes' => 'deleted',
			)
	);
	cleanup_files($box);
	$vend = ll_vendor($data['vid'],true);
	$top = form_top($data); 
	$end = form_end($data);
	return <<<HTML
$top
Box $box now deleted. $months month$s returned to {$vend['vendor']}.
$end
HTML;
}

function cleanup_files($box) {
	global $asterisk_lifeline;
	global $asterisk_rectype;
	global $asterisk_deleted;
	$dir = "$asterisk_lifeline/$box";
	$greeting = "$dir/greeting.$asterisk_rectype";
	$deleted = "$dir/greeting.$asterisk_deleted.$asterisk_rectype";
	if (is_file($greeting)) {
		@unlink($deleted);
		rename($greeting,$deleted);
	}
	$messages = "$dir/messages";
	if (($dh = @opendir($messages)) === false) return;
	while (($item = readdir($dh)) !== false) {
		$name = "$messages/$item";
		if (!is_file($name)) continue;
		if (preg_match("#\.$asterisk_deleted\.#",$name)) {
			continue;
		}
		if (!preg_match("#(.*)\.($asterisk_rectype)$#",$name,$m)) continue;
		$deleted = $m[1].".$asterisk_deleted.".$m[2];
		@unlink($deleted);
		rename($name,$deleted);
	}
	closedir($dh);
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

	$trans = ll_generate_trans($vend,'boxes');

	$top = form_top($data); 
	$end = form_end($data);
	$payment_form = payment_form($box,defpayment($vend,$months));
	$nextsubmit = htmlentities($_REQUEST['nextsubmit']);
	if (is_clients_php()) {
		$poboxes = ll_clients_poboxes($bdata['cid']);
		if (count($poboxes)) {
			$pb = array();
			$poboxfields = "";
			foreach ($poboxes as $pobox) {
				$pb[] = $pobox['box'];
				$poboxfields .= <<<HTML
<input type=hidden name="poboxes[]" value="{$pobox['box']}">
HTML;
			}
			$poboxstr = implode(",", $pb);
			$pbtitle = (count($pb) == 1 ? "po box": "po boxes");
			$poboxfield = <<<HTML
<tr>
<td><b>PO Box(es):</b></td>
<td>
<input type=checkbox name=updatepoboxes value="1" checked>
$poboxfields
Update $pbtitle $poboxstr also?
</td>
</tr>
HTML;
		} else {
			$poboxsel = poboxes('deleted_first',null,'pobox');
			$poboxfield = <<<HTML
<tr><td><b>PO Box:</b></td><td>$poboxsel</td></tr>
HTML;
		}
	}

	if (empty($bdata)) {
		ll_check_months($vend,$months);
		return <<<HTML
$top
<h3>Create New Box $box</h3>
<input type=hidden name=trans value="$trans">
<table cellpadding=3 cellspacing=0 border=0>
<tr><td><b>Box:</b></td><td><input type=hidden name=box value="$box"><b>{$vend['llphone']} Ext $box</b></td></tr>
<tr><td><b>Months:</b></td><td><input type=hidden name=months value="$months"><b>$months</b></td></tr>
<tr><td><b>{$original}Vendor:</b></td><td>{$vend['vendor']}</td></tr>
$newvendfield
</table>
$payment_form
<br>
<input type=submit name=action value="Create box" class=action>
$end
HTML;
	}

	$boxupdate = ll_check_time($vend,$box,$months);

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

	return <<<HTML
$top
<input type=hidden name=trans value="$trans">
<table cellpadding=3 cellspacing=0 border=0>
<tr><td><b>Box:</b></td><td><input type=hidden name=box value="$box"><b>{$bdata['llphone']} Ext $box</b></td></tr>
<tr><td><b>Months:</b></td><td><input type=hidden name=months value="$months"><b>$months</b></td></tr>
<tr><td><b>{$original}Vendor:</b></td><td>{$bdata['vendor']}</td></tr>
$newvendfield
<tr><td><b>Name:</b></td><td>{$bdata['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$bdata['email']}</td></tr>
<tr valign=top><td><b>Notes:</b></td><td>{$bdata['notes']}</td></tr>
<tr><td><b><nobr>Paid to:</nobr></b></td><td>{$bdata['paidto']}</td></tr>
<tr><td><b>Status:</b></td><td>{$bdata['status']}</td></tr>
$poboxfield
</table>
$payment_form
<br>
<input type=submit name=action value="$nextsubmit" class=action>
<input type=hidden name=cid value="{$bdata['cid']}">
$end
HTML;
}

function defpayment($vend,$months) {
	if (empty($vend['retail_prices'])) return 0.0;
	if (preg_match("#(?:^|;)$months=([^;]+)#",$vend['retail_prices'],$m)) {
		return sprintf('%.2f', $m[1]);
	}
	return 0.0;
}

function update_box_time($data,$months='') {
	global $table,$ldata,$phone,$script;

	$vend = ll_vendor($data['vid']);
	if ($months === '') $months = $_REQUEST['months'];

	$box = $_REQUEST['box'];
	ll_delete_trans($vend,$_REQUEST['trans'],$box);

	$bdata = ll_box($box);
	if (empty($bdata)) die("box $box does not exist!");

	if (isset($_REQUEST['personal'])) ll_update_personal($vend,$box,$_REQUEST['personal']);

	$amount = ll_update_payment($box,$ldata['vid'],$ldata['login'],$months,$_REQUEST['payment']);

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
	$poboxtr = poboxtr($bdata);
	$top = form_top($data); 
	$end = form_end($data);
	return <<<HTML
$top
$table
<tr><td><b>Box:</b></td><td><a href="$script?form=Search Boxes&search=$box">{$bdata['llphone']} Ext $box</a></td></tr>
$poboxtr
<tr><td><b><nobr>Paid to:</nobr></b></td><td>$paidto</td></tr>
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

function update_pobox_personal($data) {
	return update_personal($data,'poboxes');
}

function update_personal($data,$source='boxes') {
	global $table, $vend,$ldata,$script;
	$top = form_top($data);
	$end = form_end($data);
	$box = $_REQUEST['box'];
	if (!preg_match('#^\d+$#',$box)) 
		die("update_personal: box should be a number not $box!");

	# deal with the disorganization that is WCG International
	if ($source == 'boxes') $bdata = ll_box($box);
	else if ($source == 'poboxes') $bdata = ll_pobox($box);
	if (
		$ldata['perms'] == 'edit'
		and preg_match('#'.ll_parentpat(WCGINT).'#', $data['parent']) 
		and empty($_REQUEST['startdate'])
		and $bdata['paidto'] == 0
		and $bdata['status'] != 'deleted'
	) {
		die("WCG: you must set a paidto date for this box before making changes!");
	}

	ll_update_personal($vend,$box,$_REQUEST['personal'],$source);
	if (preg_match('#^\s*(2\d\d\d-\d\d-\d\d)#',$_REQUEST['startdate'],$m)) {
		$startdate = $m[1];
		$onemonth = date('Y-m-d',strtotime('+2 weeks'));
		if ($onemonth < $startdate) 
			die("start date $startdate can't be more than $onemonth.");
		ll_set_paidto($box,$startdate);
		$notes = "Start date added";
	} else {
		$notes = "Edited personal data";
	}

	$amount = ll_update_payment($box,$ldata['vid'],$ldata['login'],($months=0),
		array('paidon' => date('Y-m-d H:i:s'),'amount' => 0, 'notes' => $notes)
	);

	$boxrow = $statusrow = $instrrow = "";
	if ($source == 'boxes') {
		$bdata = ll_box($box);
		$poboxtr = poboxtr($bdata);
		$boxrow = <<<HTML
<tr><td><b>Box:</b></td><td><a href="$script?form=Search Boxes&search=$box">{$bdata['llphone']} Ext $box</a></td></tr>
$poboxtr
HTML;
		$statusrow = <<<HTML
<tr><td><b>Status:</b></td><td>{$bdata['status']}</td></tr>
HTML;
		$instrrow = <<<HTML
<tr><td colspan=2 align=right>
<b>PRINT THIS:</b>
<a href="index.php?box=$box&seccode={$bdata['seccode']}&llphone={$bdata['llphone']}" 
   target=_blank>instructions</a>
</td></tr>
HTML;
	} else if ($source == 'poboxes') {
		$bdata = ll_pobox($box);
		$boxrow = <<<HTML
<tr>
<td><b>PO Box:</b></td>
<td>
<a href="$script?form=Search Clients&search=pobox+$box">po box $box</a> 
&nbsp;
<a href="?action=Print+PO+Box+Reminders&pobox=$box">print reminder</a>
</td>
</tr>
HTML;
	}
	return <<<HTML
$top
$table
$boxrow
<tr><td><b>Name:</b></td><td>{$bdata['name']}</td></tr>
<tr><td><b>Email:</b></td><td>{$bdata['email']}</td></tr>
<tr valign=top><td><b>Notes:</b></td><td>{$bdata['notes']}</td></tr>
$statusrow
<tr><td><b><nobr>Paid to:</nobr></b></td><td>{$bdata['paidto']}</td></tr>
$instrrow
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
	global $permcheck,$script;
	$search = htmlentities($_REQUEST['search']);
	$box = htmlentities($_REQUEST['box']);
	if (!$search and $_REQUEST['box']) $search = htmlentities($_REQUEST['box']);
	if ($permcheck['boxes']) {
		$boxform = <<<HTML
<form name="searchform" action="$script?" method="get" style="margin-top: 8px">
Find Any Box: <input name="box" value="$box" size=5> &nbsp;
Security Code: <input name="seccode" type="password" size=5> &nbsp;
<input type=hidden name="vid" value="{$data['vid']}">
<input type=submit name=form value="Find Box">
</form>
HTML;
	}
	return <<<HTML
<form name=searchform action="$script?" method="get">
<input name="search" value="$search" size=33 style="margin-left: -5px;">
<input type=hidden name="vid" value="{$data['vid']}">
<input type=submit name=form value="Search Boxes">
</form>
<div style="margin-top: -10px;">
Boxes: &nbsp;
<a href="$script?form=Search Boxes&search=^add [0-9]* months$&vid={$data['vid']}">Show unused</a> &nbsp;&nbsp;
<a href="$script?form=Search Boxes&search=-deleted">Show active</a> &nbsp;&nbsp;
<a href="$script?form=Search Boxes&search=">Show all</a> &nbsp;&nbsp;
<a href="$script?form=Search Boxes&search=deleted&vid={$data['vid']}">Show deleted</a> &nbsp;&nbsp;
</div>
$boxform
HTML;

}

function view_boxes_form($data,$boxes=null) {
	global $table,$lightgray;
	global $vend,$permcheck,$script;
	$top = form_top($data); 
	$end = form_end($data);
	if (!is_array($boxes)) {
		$boxes = ll_boxes($vend,($showkids=true));
	}
	if ($permcheck['boxes']) $div = "&nbsp;-&nbsp;";
	else $div = ' &nbsp;&nbsp; ';
	$baseurl = "<a href=\"$script";
	$url = "$baseurl?box=";
	$numboxes = count($boxes);
	if ($numboxes <> 1) $s = 's';
	$html = <<<HTML
$numboxes record$s
$table
<tr><th><nobr>Box / Paid to</nobr></th><th>Details</th></tr>
HTML;
	foreach ($boxes as $row) {
		$paidto = $row['paidto'] == 0 ? 
			$row['status'] : $row['paidto'].' '.$row['status'];
		$box = $br = "";
		if ($row['box']) {
			$box .= "<nobr><b>{$row['box']}</b> &nbsp;&nbsp; $paidto</nobr>";
			$br = "<br>";
		}
		if ($row['pobox']) {
			$box .= "$br<nobr>PO Box <b>{$row['pobox']}</b> ".
				"&nbsp;&nbsp; {$row['pobox_paidto']}</nobr>";
		}

		if (!$row['vid']) $vid = 'n/a';
		else $vid = $row['vid'];

		if ($row['name'] == $row['pobox_name']) $name = $row['name'];
		else $name = "{$row['name']} {$row['pobox_name']}";

		if ($row['email'] == $row['pobox_email']) $email = $row['email'];
		else $email = "{$row['email']} {$row['pobox_email']}";

		if ($row['notes'] == $row['pobox_notes']) $notes = $row['notes'];
		else $notes = "{$row['notes']} {$row['pobox_notes']}";

		$vmbox = $row['box'];
		if ($row['box']) {
			$instr = <<<HTML
<a href="index.php?box=$vmbox&seccode={$row['seccode']}&llphone={$row['llphone']}"
   target=_blank>instructions</a>
HTML;
			$activity = <<<HTML
show <a href="$script?form=Call+Activity&box=$vmbox">activity</a> / 
HTML;
			# removed to avoid potential privacy complaints 
			# and to allow us to put vm and web on different servers more easily
			$msg = "$url$vmbox&listen=1\">messages</a>";
			$shsc = showcodelink($vmbox,$row['seccode']);
			$chsc = "$shsc or $url$vmbox&form=chsc\">change</a> security code";
			$edit = "$url$vmbox&search=$vmbox&form=edit\">edit</a> box";
		} else {
			$edit = $shsc = $chsc = $msg = $activity = $instr = "";
		}
		if ($row['pobox']) {
			if ($edit) $slash = "/";
			else $slash = " &nbsp; ";
			$pob = $row['pobox'];
			$edit_pobox = <<<HTML
$slash po box $baseurl?pobox=$pob&search=pobox+$pob&form=edit_pobox">edit</a>
&nbsp; $baseurl?pobox=$pob&action=Print+PO+Box+Reminders" title="print reminder for po box $pob">reminder</a>
HTML;
		} else {
			$edit_pobox = "";
		}
		if ($row['box'] and $permcheck['boxes']) {
			$add = "$url$vmbox&form=add\">add</a>";
			$sub = "$url$vmbox&form=sub\">subtract</a>";
			$del = "$url$vmbox&form=del\">delete</a>";
		} else {
			$add = $sub = $del = "";
		}
		$v = "<input type=hidden name=vendor value=\"".$row['vendor']."\">";

		if (strlen($name) > 30) $namebr = '<br>';
		else $namebr = '';
		if (strlen($notes) > 20) $notesbr = '<br>';
		else $notesbr = '';
		if (strlen($login) > 32 and $notestbr == '') $loginbr = '<br>';
		else $loginbr = '';
		if ($row['box'] and ll_has_access($data,$row)) {
			$modifystr = "$add or $sub time $div $del box $div $chsc $div";
		} else {
			$modifystr = "";
		}
		$html .= <<<HTML
<tr valign=top>
<td>$box $v</td>
<td>
<b>vendor:</b> {$vid} &nbsp;
<b>name:</b> {$name} &nbsp; 
$namebr
<b>email:</b> {$email} &nbsp; 
$notesbr
<b>notes:</b> {$notes} &nbsp;
$notesbr$loginbr
<b>last edit:</b> {$row['login']}
</td>
</tr>
<tr bgcolor=$lightgray>
<td><nobr>$activity $edit $edit_pobox &nbsp;&nbsp;</nobr></td>
<td>
<nobr>
$modifystr $instr
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
	global $table, $ldata, $min_purchase, $overdue;

	$top = form_top($data); 
	$end = form_end($data);
	if (credit_left($data['vid']) > 0 and count($overdue) == 0) {
		$vend = ll_vendor($data['vid']);
		$rate = sprintf('$%.2f',$vend['rate']);
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
	} else if (count($overdue)) {
		$html = overdueinvoices($overdue);
		return "$top$html$end";
	} else {
		return <<<HTML
$top
<blockquote>
<em>You do not have sufficient credit to purchase more voicemail.</em>
$end
HTML;
	}
}

function overdueinvoices($overdue) {
	$overduedays = INVOICEOVERDUE;
	$html = <<<HTML
<h4>The following invoices are more than $overduedays days overdue:</h4>
<i style="font-size: small">You must pay these invoices before you can purchase more voice mail time.</i>
<p>
<table cellpadding=3 cellspacing=0 width=350>
<tr>
<th>Invoice</th>
<th>Created</th>
<th>Age</th>
<th>Amount</th>
</tr>

HTML;
	foreach ($overdue as $inv) {
		$html .= <<<HTML
<tr align=center>
<td><a href="?action=invoice&invoice={$inv['invoice']}" target=_blank>{$inv['invoice']}</a></td>
<td>{$inv['created']}&nbsp;</td>
<td>{$inv['age']} days</td>
<td align=right>\$ {$inv['amount']}&nbsp;</td>
</tr>
HTML;
	}
	$html .= <<<HTML
</table>
HTML;
	return $html;
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
	global $_REQUEST,$script;
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
Invoice: <a href="$script?action=invoice&invoice=$invoice" target=_blank>$invoice</a>
$end
HTML;
}

function list_invoices($data,$showall=false,$singlepage=true) {
	global $ldata,$lightgray,$script;
	if ($data['vid'] != $ldata['vid'] and !ll_has_access($ldata,$data)) 
		die("Error: you are trying to view someone else's invoices.");
	$table = table_header(3,0,0,850);
	if ($singlepage) {
		$top = form_top($data); 
		$end = form_end($data);
	}
	$vend = ll_vendor($data['vid']);
	$invoices = ll_invoices($showall,$vend);
	if (!$singlepage and count($invoices) == 0) return false;

	$owing = ll_get_owing($data);
	$owing = $owing > 0 ? sprintf('(%.2f owing)', $owing) : '';
	$invoiced = sprintf('$%.2f',ll_get_invoiced($data));
	$vendor = $vend['vendor'];
	$squashedphone = squashedphone($vend['phone']);
	list($invtype, $invalt) = $showall ? array('All','unpaid') : array('','all');
	$html = <<<HTML
$top
<h3>$invtype Invoices for $vendor $owing</h3>
<a href="$script?form=Show+$invalt+invoices">Show $invalt</a>
<p>
HTML;
	if (count($invoices) == 0) {
		return ($html .= "<h3>No invoices</h3>\n");
	}
	$html .= <<<HTML
$table
<tr><th>invoice</th><th>vendor</th><th>created</th><th>tax</th><th>total</th><th>paid</th></tr>
HTML;
        foreach ($invoices as $invoice) {
                if (preg_match('#^0000#',$invoice['paidon'])) $invoice['paidon'] = '';
                $html .= "<tr valign=top>\n";
		$in = $invoice['invoice'];
		# if you are logged in as the parent then you can edit the invoice
		$editinv = "<td align=right><a href=\"$script?form=Edit invoice&invoice=$in\">edit</a>";
                foreach (array('invoice','vendor','created','gst','total','paidon') as $field) {
			$value = htmlentities($invoice[$field]);
                        if (is_numeric($field)) continue;
			if ($field === 'invoice') 
				$html .= <<<HTML
<td align=center><a href="$script?action=invoice&invoice=$in" target=_blank>$in</a></td>
HTML;
                        else if ($field === 'total' or $field === 'gst') {
				$html .= "<td align=right>".(sprintf('$%.2f',$value))." &nbsp;</td>";
			} else if ($field === 'paidon') {
				$html .= "<td align=right>$value $editinv</td>";
			} else if ($field === 'vendor') {
				$html .= <<<HTML
<td align=right><a href="make.php?from=admin&vid={$invoice['vid']}&action=edit">$value ({$invoice['vid']})</a></td>
HTML;
			} else $html .= "<td align=right>$value &nbsp;</td>";
                }
                $html .= "</tr>\n";
		if ($invoice['notes']) {
			$notes = htmlentities($invoice['notes']);
			$html .= "<tr bgcolor=$lightgray><td colspan=7 align=center><b>Notes:</b> $notes</td></tr>\n";
		}
        }
	$html .= <<<HTML
</table>
$end
HTML;
	return $html;
}

function edit_invoice($invoice) {
	global $ldata,$script;
	if (!preg_match('#^\d+$#', $invoice)) die("invoice should be a number!");

	if (!preg_match('#invoices|^s$#',$ldata['perms'])) 
		die("you do not have permission to view invoices!");

	$idata = ll_invoice($invoice);
/*
	# see the paidon row below
	if ($ldata['vid'] == $idata['vdata']['parent']) {
		die("you do not have permission to edit this invoice!");
	}
*/
	
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
<a href="$script?action=invoice&invoice=$invoice" target=_blank>$invoice</a>
for 
<a href="$script?vid={$idata['vid']}&form=Show all invoices">{$idata['vdata']['vendor']}</a></h3>
<input type=hidden name=invoice value="$invoice">
<input type=hidden name=vid value="{$idata['vid']}">
$table
<tr><td>invoice</td><td>{$idata['invoice']}</td></tr>
<tr><td>vendor</td><td>{$idata['vdata']['vendor']}</td></tr>
<tr><td>created</td><td>{$idata['created']}</td></tr>
<tr><td>tax</td><td>{$idata['gst']}</td></tr>
<tr><td>total</td><td>{$idata['total']}</td></tr>
HTML;
	if ($ldata['vid'] == $idata['vdata']['parent']) {
		$html .= <<<HTML
<tr><td>paid on</td>
    <td><input id="paidon" name="paidon" value="{$idata['paidon']}" max=10 size=10> 
	YYYY-MM-DD &nbsp;
	<a class=support href="javascript:void(0);" 
	   onclick="topform.paidon.value='$today'; return false;">set to today</a> &nbsp;
	<a class=support href="javascript:void(0);" 
	   onclick="topform.paidon.value=''; return false;">clear</a>
</td></tr>
HTML;
	} else {
		$html .= <<<HTML
<tr><td>paid on</td><td>{$idata['paidon']} &nbsp;</td></tr>
HTML;
	}
	$html .= <<<HTML
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
	global $ldata,$script;
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
