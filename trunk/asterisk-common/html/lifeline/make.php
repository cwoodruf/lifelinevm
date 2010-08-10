<?php
# purpose of this script is to make other lifeline vendors
require_once("php/globals.php");
require_once("$lib/pw/auth.php");
require_once("php/forms.php");

$action = $_REQUEST['action'];
if ($action === 'logout') delete_login();
$login = $_REQUEST['login'];
if (!preg_match('#^[\w\.\@-]*$#',$login)) die("bad login value $login for adding or updating user!");

# remember where we came from
$_SESSION['from'] = $from = 
		$_REQUEST['from'] == 'admin' ? 'admin' : 
			($_SESSION['from'] == 'admin' ? 'admin' : null);

# log in and check differently depending on where we came from
# this is more of a courtesy than a security check
# your login credentials really determine what you can do
if ($_REQUEST['login'] != 'superuser' and $from == 'admin') {
	$ldata = login_response($_SERVER['PHP_SELF'],"/lifeline/admin.php",'ll_pw_auth');
	if ($ldata['perms'] != 's') {
		if (!preg_match('#vendors|logins#',$ldata['perms'])) 
			die("This area is restricted access!");
	}
	$vend = $sdata = ll_vendor($ldata['vid']);
	$vid = $ldata['vid'];

	# make a compressed ancestry identifier like parentvid:myvid
	$parentparts[$sdata['parent']] = true;
	$parentparts[$vid] = true;
	$defparent = implode(":",array_keys($parentparts));

	if (isset($_REQUEST['vid'])) {
		$vid = $_REQUEST['vid'];
		if (!preg_match('#^\d*$#',$vid)) die("bad vendor id $vid!");
		if (isset($vid)) $vend = ll_vendor($vid);
		else unset($vend);
	}

	$goback = "<a href=\"admin.php\">Back to voicemail admin page</a>";
} else {
	# use 'redirect.php' as the login script if we are doing a direct login
	$ldata = login_response('redirect.php',$_SERVER['PHP_SELF'],'ll_superuser');
	if ($ldata['vid'] != 0) die("No direct access! <a href=\"admin.php\">Back to admin</a>");

	$sdata = ll_vendor($ldata['vid']);
	$defparent = isset($ldata['parent']) ? $ldata['parent'] : 0;

	$vid = $_REQUEST['vid'];
	if (!preg_match('#^\d*$#',$vid)) die("bad vendor id $vid!");
	if (isset($vid)) $vend = ll_vendor($vid);
	else unset($vend);
	$goback = "<a href=/lifeline/make.php?action=logout>Log out</a>";

}
$seller = $sdata['vendor'];

# create pages
if ($action === 'invoice') {
	print invoice($vend);
	exit;
}
if ($sdata['perms'] == 's') $myperms = $baseperms;
else $myperms = split(':',$ldata['perms']);
foreach ($myperms as $p) {
	$permcheck[$p] = true;
}

if ($permcheck['vendors']) 
	$emaillistlink = "<a href=/lifeline/make.php?from=$from>Main accounts and users page</a>";
print <<<HTML
<html>
<head>
<title>$seller admin</title>
<link rel=stylesheet type=text/css href=/lifeline/css/admin.css>
</head>
<body bgcolor=lightyellow>
<center>
$goback
<h3>$seller admin</h3>
$emaillistlink
<p>

HTML;

if ($sdata['vid'] == 0) {
	print <<<HTML
<form action=/lifeline/make.php method=get>
$seller 
<input type=submit name=action value="Create new vendor"> &nbsp;&nbsp;
<input type=hidden name=parent value="$defparent">
<input type=submit name=action value="View paid invoices"> &nbsp;&nbsp;
<input type=submit name=action value="View invoices"> &nbsp;&nbsp;
<input type=submit name=action value="Email list">
</form>

HTML;
} else if ($permcheck['logins'] or $permcheck['vendors']) {
	print <<<HTML
<form action=/lifeline/make.php method=get>
$seller 
<input type=hidden name=from value="$from">
<input type=submit name=action value="Update info"> &nbsp;&nbsp;
<input type=hidden name=vid value="$vid">
<input type=hidden name=parent value="$defparent">

HTML;
	if ($permcheck['logins']) {
		print <<<HTML
<input type=submit name=action value="New login user"> &nbsp;&nbsp;
<input type=submit name=action value="Show logins"> &nbsp;&nbsp;

HTML;
	}
	if ($permcheck['vendors']) {
		print <<<HTML
<input type=submit name=action value="Create new vendor"> &nbsp;&nbsp;

HTML;
	}
	print "</form>\n";
}

print <<<HTML
<br>
<form action=/lifeline/make.php method=post>
<input type=hidden name=from value="$from">

HTML;

if ($action === 'Create new vendor') vendor_form(null);
else if ($action === 'new_user' or $action === 'New login user') 
	user_form(is_array($_REQUEST['vend']) ? $_REQUEST['vend']: $vend);
else if ($action === 'emails' or $action === 'Email list') show_emails();
else if ($action === 'Add user') add_user($vend); 
else if ($action === 'modify_user') user_form($vend,$login);
else if ($action === 'Update user') { ll_del_user($vend,$login); add_user($vend); }
else if ($action === 'delete_user') del_user_form($vend);
else if ($action === 'Really delete user') del_user($vend);
else if ($action === 'del_vendor') del_vendor_form($vend);
else if ($action === 'Really delete vendor') del_vendor($vend);
else if ($action === 'Show logins' or $action === 'show_logins') show_logins($vend);
else if ($sdata['vid'] === 0 and $action === 'View invoices') invoices_form();
else if ($sdata['vid'] === 0 and $action === 'View paid invoices') invoices_form('paid');
else if ($sdata['vid'] === 0 and $action === 'Indicate invoice paid') pay_invoices();
else if ($action === 'Update vendor') update_vendor('update',$vend);
else if ($action === 'Create vendor') update_vendor('insert');
else if ($action === 'transfer_box') transfer_box_form();
else if ($action === 'Confirm box transfer') transfer_box_confirm();
else if ($action === 'Really transfer box') transfer_box();
else if (
	(isset($vid) and $sdata['vid'] === 0) or 
	$action === 'edit' or 
	$action === 'Update info') vendor_form($vend);
else list_vendors($_REQUEST['findvid']);

print <<<HTML
</form>
</center>
</body>
</html>
HTML;

# end output

#================================== functions ================================#
function transfer_box_form() {
	global $ldata, $permcheck;
	if ($ldata['vid'] != 0) {
		if (!$permcheck['boxes']) 
			die("you do not have permission to transfer a box!");
	}

	$vid = $_REQUEST['vid'];
	if (!preg_match('#^\d+$#',$vid)) die("vendor id should be a number!");
	$currvendor = ll_vendor($vid);
	if (!$currvendor) die("vendor $vid does not exist!");

	$boxes = ll_boxes($vid,($showkids=false),'','order by box asc');
	if ($boxes) {
		$boxsel = "<select name=boxsel><option>\n";
		foreach ($boxes as $bdata) {
			$boxsel .= 
				"<option value=\"{$bdata['box']}\">{$bdata['box']} {$bdata['status']}</option>\n";
		}
		$boxsel .= "</select>\n";
	}
	$vendors = ll_vendors($ldata['vid']);

	print <<<HTML
<h3>Transfer a box from one vendor to another</h3>
<table cellpadding=3 cellspacing=0>
<tr>
<td>
Select a Box: <input name=box size=5> $boxsel
<p>
Current vendor: <b>{$currvendor['vendor']}</b>
<input type=hidden name="currvid" value="{$currvendor['vid']}">
<p>
New vendor: <select name="newvid">
<p>

HTML;
	foreach ($vendors as $vendor) {
		if ($vendor['vid'] == $vid) $checked = 'selected';
		else $checked = '';
		print "<option value=\"{$vendor['vid']}\" $checked>{$vendor['vendor']}</option>\n";
	}
	print <<<HTML
</select>
<p>
<center><input type=submit name=action value="Confirm box transfer"></center>
</td>
</tr>
</table>

HTML;

}

function transfer_box_confirm() {
	global $ldata, $permcheck;

	list($box,$currvendor,$newvendor) = get_transferdata();
	if ($ldata['vid'] != 0) {
		if (!$permcheck['boxes'])
			die("You do not have permission to transfer!");
	}

	print <<<HTML
<input type=hidden name=currvid value={$currvendor['vid']}>
<input type=hidden name=newvid value={$newvendor['vid']}>
<input type=hidden name=box value=$box>
<h3>Transfer box $box from 
    {$currvendor['vendor']} ({$currvendor['vid']})
to {$newvendor['vendor']} ({$newvendor['vid']})?
<p>
<input type=submit name=action value="Really transfer box">

HTML;

}

function transfer_box() {
	global $ldata, $permcheck;

	list($box,$currvendor,$newvendor) = get_transferdata();
	if ($ldata['vid'] != 0) {
		if (!$permcheck['boxes']) 
			die("you do not have permission to transfer a box!");
	}

	if (ll_transferbox($box,$currvendor,$newvendor))  $result = "transferred";
	else $result = "not tranferred (error)";

	print <<<HTML
<h3>Box $box $result from {$currvendor['vendor']} to {$newvendor['vendor']}</h3>

HTML;
}

function get_transferdata() {
	$box = $_REQUEST['box'];
	if (empty($box)) $box = $_REQUEST['boxsel'];
	if (!preg_match('#^\d+$#', $box)) die("box $box should be a number!");
	$bdata = ll_box($box);
	if (!$bdata) die("box $box does not exist!");

	$currvid = $_REQUEST['currvid'];
	if (!preg_match('#^\d+$#',$currvid)) die("old vendor id should be a number!");

	$newvid = $_REQUEST['newvid'];
	if (!preg_match('#^\d+$#',$newvid)) die("new vendor id should be a number!");

	if ($currvid == $newvid) die("current vendor and selected new vendor are the same for box $box!");

	$currvendor = ll_vendor($currvid);
	if (!$currvendor) die("selected current vendor could not be found!");
	if ($currvendor['status'] == 'deleted') die("current vendor $currvid deleted!");
	if ($bdata['vid'] != $currvendor['vid']) die("box $box does not belong to vendor $currvid!");

	$newvendor = ll_vendor($newvid);
	if (!$newvendor) die("selected new vendor could not be found!");
	if ($newvendor['status'] == 'deleted') die("new vendor $newvid deleted!");

	if ($currvendor['parent'] != $newvendor['parent'] 
		and !ll_has_access($currvendor['vid'],$newvendor) 
		and !ll_has_access($newvendor['vid'],$currvendor)
	) 
		die("{$newvendor['vendor']} is not part of the same group as {$currvendor['vendor']}.");

	return array($box, $currvendor, $newvendor);
}

function show_emails() {
	global $ldata,$from;
	$vendors = ll_vendors($ldata['vid']);
	if ($vendors === false) return;
	print "<h3>Vendor emails</h3>\n<table cellpadding=5 cellspacing=0><tr><td>\n";
	foreach ($vendors as $v) {
		if ($seen[$v['email']]) continue;
		if (preg_match('#deleted#',$v['status'])) continue;
		if (!strstr($v['email'],'@')) continue;
		$seen[$v['email']] = true;
		$v['email'] = preg_replace('#\s+(\S)#',",<br>\n$1",$v['email']);
		print "{$v['email']},<br>\n";
	}
	print "</td></tr></table>\n";
}

function list_vendors($vid=null) {
	global $ldata,$from;
	if (preg_match('#^\d+$#',$vid)) {
		if (!ll_has_access($ldata,ll_vendor($vid))) die("you do not have access to this vendor!");
		$vendors[] = ll_vendor($vid);
	} else {
		$vendors = ll_vendors($ldata['vid']);
	}
	if ($vendors === false) return;
	$make = "/lifeline/make.php";
	$div = ''; # "&nbsp;-&nbsp;";
	print <<<HTML
<table cellpadding=5 cellspacing=0 border=0 width=1200>
<tr><th>id</th><th>vendor</th><th>invoiced</th><th>owing</th><th>months</th><th>tools</th></tr>
HTML;
	$owing = ll_get_owing();
	$invoiced = ll_get_invoiced();
	foreach ($vendors as $vend) {
		$vendor = $vend['vendor'];
		$vid = $vend['vid'];

		if ($owing[$vid]) $owed = sprintf('$%.2f',$owing[$vid]);
		else $owed = '&nbsp;';

		if ($invoiced[$vid]) $invstr = sprintf('$%.2f',$invoiced[$vid]); 
		else $invstr = '&nbsp;';

		$created = preg_replace('# .*#','',$vend['created']);
		$boxcount = ll_activeboxcount($vid);
		$logincount = ll_logincount($vid);
		if (
			($ldata['perms'] == 's' or strpos($ldata['perms'],'vendors') !== false)
			and $vid != $ldata['vid'] # don't delete yourself!
			and $owed == 0 and $boxcount == 0) 
		{
			$del_vendor = "<a href=\"$make?action=del_vendor&vid=$vid\">delete</a>";
		}
		else 
		{
			$del_vendor = '<span style="color: gray">delete</span>';
		}
		if (!$boxcount) $boxcount = 'no';
		if ($boxcount <> 1) $es = 'es';
		else $es = '';
		if (!$logincount) $logincount = 'no';
		if ($logincount <> 1) $s = 's';
		else $s = '';
		if ($vid != $ldata['vid']) {
			$loginlink = "<a href=\"admin.php?switch_vendor=$vid\">log in</a>";
		} else {
			$loginlink = "<span class=\"inactivelink\">log in</span>";
		}
		$squashedphone = preg_replace('#.*?((?:\d\D*){10}).*#',"$1",$vend['phone']);
		$squashedphone = preg_replace('#\D#','',$squashedphone);
		print <<<HTML
<tr>
<td>$vid</td>
<td>$vendor</td>
<td align=right><a href="admin.php?vid=$vid&form=Show all invoices">$invstr</a></td>
<td align=right><a href="admin.php?vid=$vid&form=Show unpaid invoices">$owed</a></td>
<td align=right>$vend[months]</td>
<td>
<table cellspacing=0 cellpadding=2 border=0 style="border: none">
<tr>
<td width=100>
$loginlink / 
<a href="$make?action=edit&from=$from&vid=$vid">edit</a> $div
</td>
<td width=120>
<nobr>
<a href="$make?action=show_logins&from=$from&vid=$vid">$logincount login$s</a>
(<a href="$make?action=new_user&from=$from&vid=$vid">add</a>) $div
</nobr>
</td>
<td width=200>
<nobr>
<a href="admin.php?form=View your voicemail boxes&vid=$vid&showkids=0">$boxcount box$es</a>
(<a href="$make?action=transfer_box&from=$from&vid=$vid">transfer box</a>) $div
</nobr>
</td>
<td width=80>
$del_vendor
</td>
</tr>
</table>
</td>
</tr>
<tr bgcolor=lightgray><td colspan=6>
<b>Contact:</b> <a href="sip:1$squashedphone@192.168.1.44">{$vend['phone']}</a> &nbsp;&nbsp; {$vend['contact']} {$vend['email']} &nbsp;&nbsp; {$vend['notes']}
</td></tr>
HTML;
	}
}

function vendor_form($vend) {
	global $seller,$from,$ldata,$sdata,$def_credit_limit,$defparent;

	$vendor = $vend['vendor'];
	$vid = $vend['vid'];
	print "<h3>Edit Vendor $vendor</h3>\n";
	if ($ldata['vid'] != $vid) {
		$loginbuttons = <<<HTML
<input type=submit name=action value="Show logins"> &nbsp;&nbsp;
<input type=submit name=action value="New login user"> &nbsp;&nbsp;

HTML;
	}

	if (isset($vend)) {
		print <<<HTML
$vendor 
<input type=reset value=Reset> &nbsp;&nbsp;
$loginbuttons
<input type=hidden name="vend[parent]" value="$defparent">

HTML;
		$submitname = 'Update';
	} else $submitname = 'Create';

	if (!isset($vend['vid'])) $cl = $def_credit_limit;
	else $cl = $vend['credit_limit'];

	if ($ldata['vid'] == ROOTVID) {
		if ($vend['parent'] == "") $parent = ROOTVID;
		else $parent = $vend['parent'];
		$parent = <<<HTML
<tr><td> parent </td><td><input name="vend[parent]" size=60 value="$parent"></td></tr>
HTML;
		$creditlimit = <<<HTML
<tr><td> credit limit </td><td><input name="vend[credit_limit]" value="$cl"> <i>-1 = unlimited</i></td></tr>
HTML;
		if ($vend['rate'] == "") $rate = DEFRATE;
		else $rate = $vend['rate'];
		$rateform = <<<HTML
<tr><td> rate </td><td> <input name="vend[rate]" value="$rate"></td></tr> 
HTML;
		if ($vend['llphone'] == "") $llphone = DEFPHONE;
		else $llphone = $vend['llphone'];
		$llphoneform = <<<HTML
<tr><td> incoming phone </td><td> <input name="vend[llphone]" value="$llphone"></td></tr> 
HTML;
	} else {
		if ($vend['parent'] != '') {
			$parent = <<<HTML
<tr>
<td> parent </td><td>{$vend['parent']} 
    <input type=hidden name="vend[parent]" value="{$vend['parent']}">&nbsp;
</td>
</tr>
HTML;
		} else {
			$parent = <<<HTML
<tr>
<td> parent </td><td>$defparent 
    <input type=hidden name="vend[parent]" value="$defparent">&nbsp;
</td>
</tr>
HTML;
		}
		$clvalue = $cl > 0 ? "$cl month".($cl > 1 ? 's':'') : "";
		$creditlimit = <<<HTML
<tr><td> credit limit </td><td>$clvalue &nbsp; </td></tr>
HTML;
	}

	if ($vend['gstexempt']) $nogst = 'checked'; else  $hasgst = 'checked';
	print <<<HTML
<input type=submit name=action value="$submitname vendor"> 
<p>
<input type=hidden name="vid" value="$vid">
<input type=hidden name="vend[vid]" value="$vid">
<table cellpadding=5 cellspacing=0 border=0>
$llphoneform
<tr><td> vendor </td><td><input name="vend[vendor]" size=60 value="$vendor"></td></tr>
<tr><td> notes </td><td><input name="vend[notes]" size=60 value="{$vend['notes']}"></td></tr>
<tr><td> created </td><td> {$vend['created']} </td></tr>
$parent
<tr><td> address </td><td><input name="vend[address]" size=60 value="{$vend['address']}"></td></tr>
<tr><td> phone </td><td><input name="vend[phone]" size=60 value="{$vend['phone']}"></td></tr>
<tr><td> contact </td><td><input name="vend[contact]" size=60 value="{$vend['contact']}"></td></tr>
<tr><td> email </td><td><input name="vend[email]" size=60 value="{$vend['email']}"></td></tr>
<tr><td> fax </td><td><input name="vend[fax]" size=60 value="{$vend['fax']}"></td></tr>
<tr><td> gst exempt </td><td> 
	<input type=radio name=gstexempt value=0 $hasgst> No 
	<input type=radio name=gstexempt value=1 $nogst> Yes</td></tr>
$rateform
<tr><td> months </td><td> {$vend['months']} </td></tr>
<tr><td> gst number </td><td><input name="vend[gst_number]" size=60 value="{$vend['gst_number']}"></td></tr>
$creditlimit
<tr><td> status </td><td> {$vend['status']} </td></tr>
</table>
HTML;
}

function update_vendor($dbaction,$vend=null) {
	global $from, $schema, $action;

	$newvend = $_REQUEST['vend'];
	if (isset($vend)) {
		$vid = $vend['vid'];
		$vendor = $vend['vendor'];
	} else {
		$vid = $newvend['vid'];
		$vendor = $newvend['vendor'];
	}
	foreach ($newvend as $name => $value) {
		if (!isset($schema['vendors'][$name])) continue;
		if ($name === 'vid' and $dbaction === 'insert') continue;
		# see if the value is different from what is in the db
		if (is_array($vend)) {
			if ($vend[$name] != $value) $update[$name] = $value;
		} else {
			$update[$name] = $value;
		}
	}
	if ($update['parent']) $parent = ll_vendor($update['parent']);
	if (empty($update['llphone'])) {
		if ($parent['llphone']) $update['llphone'] = $parent['llphone'];
		else $update['llphone'] = DEFPHONE;
	}
	if (empty($update['rate'])) {
		if ($parent['rate']) $update['rate'] = $parent['rate'];
		else $update['rate'] = DEFRATE;
	}
	if (count($update)) {
		if ($dbaction === 'update') ll_save_to_table('update','vendors',$update,'vid',$vid,true);
		else if ($dbaction === 'insert') ll_save_to_table('insert','vendors',$update,null,$vid,true);
	}
	$update = ll_vendor($vid);
	print <<<HTML
<h3>Updated Vendor 
    <a href="make.php?action=edit&from=$from&vid=$vid">{$update['vendor']}</a>
</h3>
HTML;
}

function show_logins($vend) {
	global $from;
	$vendor = $vend['vendor'];
	$vid = $vend['vid'];
	print "<h3>User list for Vendor $vendor</h3>\n";
	if ($vend['vid'] == 0) return;
	$logins = ll_users($vend);
	print <<<HTML
<input type=submit name=action value="New login user"><p>
<input type=hidden name=vid value="$vid">
<table cellpadding=5 cellspacing=0 border=0>
HTML;
	foreach($logins as $user) {
		$count++;
		$login = urlencode($user['login']);
		print <<<HTML
<tr>
<td>$user[login]</td><td>$user[created]</td><td>$user[perms]</td>
<td>
<a href="$make?action=modify_user&from=$from&vid=$vid&login=$login">modify user</a> &nbsp;&nbsp;
<a href="$make?action=delete_user&from=$from&vid=$vid&login=$login">delete user</a>
</td>
</tr>
HTML;
	}
	print "<table>\n";
	if (!$count) print "No logins<br>\n";
}

function user_form($vend,$login=null) {
	global $from,$ldata,$baseperms,$permhelp;
	$vendor = $vend['vendor'];
	$vid = $vend['vid'];
	if ($login == null) {
		print "<h3>Add user (Vendor $vendor)</h3>\n";
		$perms['edit'] = 'checked';
		$action = 'Add';
	} else {
		$user = ll_pw_data($login);
		$action = 'Update';
		$passwordentry = <<<HTML
<tr>
<td align=center colspan=2>
<a href="{$_SERVER['PHP_SELF']}?action=modify_user&from=$from&edpass=1&login=$login&vid=$vid">
Change password</a>
</td>
</tr>
HTML;
		foreach(explode(':',$user['perms']) as $p) {
			$perms[$p] = 'checked';
		}
	}
	# avoid permissions escalation by using the logged in user's permissions as a template
	if ($ldata['perms'] == 's') $permlist = $baseperms;
	else $permlist = explode(':',$ldata['perms']); 

	foreach ($permlist as $p) {
		if (!in_array($p,$baseperms)) continue;
		$permsblock .= <<<HTML
<span title="{$permhelp[$p]}">
<input type=checkbox name="perms[$p]" value="1" {$perms[$p]} > $p &nbsp;
</span>

HTML;
	}
	if ($login == null or $_REQUEST['edpass']) {
		$passwordentry = <<<HTML
<tr><td>Password:</td><td> <input type=password name=password1 size=40></td></tr>
<tr><td>Password again:</td><td> <input type=password name=password2 size=40></td></tr>
HTML;
	}
	if ($ldata['vid'] != $vid) {
		$loginbuttons = <<<HTML
$vendor 
<input type=submit name=action value="Show logins"> &nbsp;&nbsp;
<input type=submit name=action value="New login user">

HTML;
	}
	$notes = htmlentities($user['notes']);
	print <<<HTML
$loginbuttons
<br>
<br>
<input type=hidden name=vid value="$vid">
<table cellpadding=5 cellspacing=0 border=0>
<tr><td colspan=2 align=center><input type=reset value=Reset></td></tr>
<tr><td>Login/email:</td><td><input name=login size=40 value="$login"></td></tr>
$passwordentry
<tr valign=top>
    <td>Permissions:<div style="font-size: small">(hover mouse for detail)</div></td><td>
	<nobr style="background: lightyellow">
	$permsblock
	</nobr>
	&nbsp;&nbsp;
</i></nobr></td></tr>
<tr><td>Notes:</td><td><input name=notes size=40 value="$notes"></td></tr>
<tr><td colspan=2 align=center><input type=submit name=action value="$action user"></td></tr>
</table>
HTML;
}

function add_user($vend) {
	global $ldata, $baseperms,$from;
	$vendor = $vend['vendor'];
	$login = $_REQUEST['login'];
	$vid = $vend['vid'];
	$password = $_REQUEST['password1'];
	if ($password !== $_REQUEST['password2']) die("passwords don't match!");

	# avoid privelage escalation by using the currently logged in user as the permissions template
	if ($ldata['perms'] == 's') $perms = $baseperms;
	else $perms = explode(':',$ldata['perms']); 

	foreach ($perms as $p) {
		if (!in_array($p,$baseperms)) continue;
		if ($_REQUEST['perms'][$p]) $permlist[$p] = true;
	}
	if (is_array($permlist)) $permstr = implode(':',array_keys($permlist));

	$notes = $_REQUEST['notes'];
	ll_add_user($vend,$login,$password,$permstr,$notes);
	print <<<HTML
<h3>Updated user <a href="make.php?action=modify_user&from=$from&vid=$vid&login=$login">$login</a>. 
<a href="make.php?action=show_logins&from=$from&vid=$vid">Logins for $vendor</a>
</h3>
HTML;

}

function del_user_form($vend) {
	global $_REQUEST;
	$vendor = $vend['vendor'];
	$vid = $vend['vid'];
	$login = $_REQUEST['login'];
	$udata = ll_pw_data($login);
	if ($udata['vid'] != $vend['vid']) die("Vendor $vendor does not have a user $login!"); 
	print "<h3>Delete user $login? (Vendor $vendor)</h3>\n";
	print <<<HTML
<input type=hidden name=login value="$login">
<input type=hidden name=vid value="$vid">
<input type=submit name=action value="Really delete user">
HTML;
}

function del_user($vend) {
	$vendor = $vend['vendor'];
	$login = $_REQUEST['login'];
	ll_del_user($vend,$login);
	print "<h3>Deleted user $login (Vendor $vendor)</h3>\n";
}

function check_vendor_delete($vid) {
	global $ldata;
	if (!preg_match('#^\d+$#',$vid)) 
		die("del_vendor_form: no vendor id!");
	if ($ldata['vid'] == $vid) 
		die("del_vendor_form: can't delete your own vendor.");
	if ($ldata['perms'] != 's' and strpos($ldata['perms'],'vendors') === false) 
		die("del_vendor_form: no privelages to delete vendors.");
	return true;
}

function del_vendor_form() {
	$vid = $_REQUEST['vid'];
	if (!check_vendor_delete($vid)) return;
	$vend = ll_vendor($vid);
	print <<<HTML
<h3>Delete vendor {$vend['vendor']}?</h3>
<input type=hidden name=vid value="$vid">
<input type=submit name=action value="Really delete vendor">
HTML;
}

function del_vendor() {
	$vid = $_REQUEST['vid'];
	if (!check_vendor_delete($vid)) return;
	$vend = ll_vendor($vid);
	ll_del_vendor($vid);
	print "<h3>Deleted vendor {$vend['vendor']}</h3>\n";
}

function invoices_form($show='') {
	if ($show === 'paid') $invoices = ll_invoices(true);
	else $invoices = ll_invoices();
	print <<<HTML
<h3>View $show invoices</h3>
<input type=submit name=action value="Indicate invoice paid"><p>
<table cellpadding=5 cellspacing=0 border=0>
<tr><th>&nbsp;</th><th>invoice</th><th>vendor</th><th>created</th><th>gst</th><th>total</th><th>paid</th></tr>
HTML;
	foreach ($invoices as $invoice) {
		if ($invoice['paidon'] == '') {
			$invoice['paidon'] = 'unpaid';
			$checkbox = "<input type=checkbox name=\"invoices[]\" value=\"".$invoice['invoice']."\">";
		} else $checkbox = '';
		print "<tr><td>$checkbox</td>\n";
		$link = "<a href=\"make.php?action=invoice&invoice={$invoice['invoice']}\">";
		foreach ($invoice as $field => $value) {
			if (is_numeric($field)) continue;
			print "<td>$link$value</a> &nbsp;</td>";
		}
		print "</tr>\n";
	}
	print "</table>\n";
}

function pay_invoices() {
	global $ldata;
	$invoices = $_REQUEST['invoices'];
	ll_pay_invoices($invoices,$ldata);
	print "<h3>Invoices updated</h3>";
}

