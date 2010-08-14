<?php
$amounts = array( '4' => '10', '6' => '12', '12' => '22' );
$hst = .12;

$months = $_REQUEST['months'];
if (!isset($amounts[$months])) $months = 4;

$net = sprintf('%.2f',$amounts[$months]);
$hst = "<i>included</i>"; 
$total = sprintf('$ %.2f',$net + $hst);

?>

<html>
<head>
<title>Lifeline Voicemail: pay by mail</title>
<style>
.inputtext {
	padding-bottom: 10px;
}
</style>
</head>
<body vlink=#660099 link=#686868 alink=#686868 bgcolor=lightyellow>
<table cellpadding=10><tr>
<td>
<h1>Lifeline Voicemail: pay by mail</h1>
<?php if ($_REQUEST['action']) { ?>
<i>Print this form and mail it to:</i>
<?php } ?>
<br>
<br>
<br>
<p>
<b>Lifeline Voice Mail</b><br>
142 - 757 West Hastings Street,<br>
Vancouver, BC<br>
V6C 1A1<br>
<br>
<br>
<br>
<br>
<p>
<form name="mailin" action="pay.php" method=get>
<input type=hidden name=months value="<?php print htmlentities($months); ?>">
<table cellpadding=5 cellspacing=0 border=1>
<tr><td>Date:</td><td><?php print date('Y-m-d'); ?></td></tr>
<tr valign=top><td>Your Name:</td><td width=320>
<?php 
if ($_REQUEST['action']) { 
	print htmlentities($_REQUEST['name']); 
} else { 
	print "<input type=text class=\"inputtext\" name=name size=35>";
}
?>&nbsp;
</td></tr>
<tr valign=top><td>Your Box Number:</td><td>
<?php 
if ($_REQUEST['action']) { 
	print htmlentities($_REQUEST['box']); 
} else { 
	print "<input type=text class=\"inputtext\" name=box value=\"new subscription\" size=35>";
}
?>&nbsp;
</td></tr>
<tr><td>Time:</td><td align=right><?php print $months; ?> months</td></tr>
<tr><td>Amount:</td><td align=right><?php print $net; ?></td></tr>
<tr><td>HST:</td><td align=right><?php print $hst; ?></td></tr>
<tr><td>Total:</td><td align=right><b><?php print $total?></b></td></tr>
</table>
<p>
<?php if (!$_REQUEST['action']) { ?>
<input type=submit name=action value="Show printable form">
<?php } ?>
</form>
<p>
<i>We accept <b>postal money orders</b> for payment by mail.</i>
</td>
</tr></table>
</body>
</html>
