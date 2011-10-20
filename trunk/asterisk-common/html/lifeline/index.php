<?php 
session_start();
require_once('php/globals.php'); 
@include_once("$lib/mysql.php"); 
if (!function_exists(ll_has_access)) {
	function ll_has_access($you,$me) { return true; }
	function ll_showcode($code) { return; }
	function ll_box($box) { return; }
}
if ($_REQUEST['llphone']) 
	$llphone = $_REQUEST['llphone'];
else $llphone = $phone;

if (isset($_SESSION['login']['login'])) $printedby = md5($_SESSION['login']['login']);

if (!($vend = ll_vendor($_SESSION['login']['vid']))) {
	unset($_REQUEST);
}
if ($_REQUEST['amount']) {
	$title = "Proof of Payment / Instructions";
} else {
	$title = "Lifeline: Instructions";
}
?>
<html>
<head>
<title><?php print $title; ?></title>
<link rel=stylesheet type=text/css href=css/base.css>
<link rel=stylesheet type=text/css href=css/admin.css>
<style type=text/css>
li {
	padding-bottom: 10px;
}
i, i * {
	font-size: small;
}
</style>
</head>
<body>
<center>
<h3><?php print $title; ?></h3>
<table width=540 cellpadding=3 cellspacing=0 border=0>
<tr><td>
<h4>Date: <?php print date('Y-m-d'); ?></h4>
<p>
<nobr>
Your phone number is &nbsp; 
<span style="text-decoration: underline; font-weight: bold;">
<?php print $llphone; ?> 
Ext <?php print $_REQUEST['box']  ? ($box=htmlentities($_REQUEST['box'])) : '_______'; ?>
</span> &nbsp;
(Security code:
<?php print $_REQUEST['seccode']  ? ($sc=htmlentities(ll_showcode($_REQUEST['seccode']))) : '_______'; ?>)
<?php if ($_REQUEST['amount']) : ?>
<p>
Payment: 
$<?php print sprintf('%.2f',$_REQUEST['amount']) ?>
</nobr>
<p>
<div style="padding-top: 30px;">
Received by: __________________________________________
</div>
<div style="font-size: small; font-style: italic">
This is your receipt and proof of purchase.<br>
Please keep this receipt in case of problems with your account.<br>
</div>
<div style="font-size: x-small; float: right;"><?php print $printedby; ?></div>
<?php endif; ?>
<?php if ($bdata['paidto'] > 0) : ?>
<div>
<i>Paid to: <?php print $bdata['paidto']; ?></i>
</div>
<?php endif; ?>
<?php if ($bdata['status']) : ?>
<div>
<i>Status: <?php print $bdata['status']; ?></i>
</div>
<?php endif; ?>
<div style="padding-top: 8px;">
<i><b>Rates:</b> $3.00 per month, $10 for four months, $25 per year. <br>
<b>Contact:</b> 604 248-4930 Ext 6040 &nbsp;&nbsp;
<b>Website:</b> http://callbackpack.com</i>
</div>

<h4>Using voicemail</h4>
<h4>Leaving messages</h4>
<ol>
<li>Dial <b><?php print $llphone; ?></b>
<li>Enter the box number <b><?php print $box; ?></b> to leave a message
<li>You can skip the greeting by pressing #
<li>Leave a message
<li><b>To leave another message</b> press a key and wait for the prompt
</ol>
<h4>Accessing your box for the first time</h4>
<ol>
<li>Dial <b><?php print $llphone; ?></b>
<li>Enter the box number <b><?php print $box; ?></b>
<li>Enter the security code <b><?php print $sc; ?></b> to hear the main menu:
<ul>
<li>Press 1 from the main menu to listen to messages
<li>Press 2 from the main menu to listen to your greeting
<li><b>Press 3 from the main menu to record your greeting</b>
<li>Press 4 from the main menu to get your subscription date
<li><b>Press # from the main menu to change your security code</b>
<li>Press * from the main menu to go back to the start<br>
<i>You can go back to the start up to 10 times in one call.</i>
</ul>
</ol>

<h4 style="page-break-before: always;">Listen to messages</h4>
<i>When you press 1 from the main menu you will hear the number of messages you have <br>
and your <b>most recent</b> message followed by the listen to messages menu:</i>
<ul>
<li>Press 1 to listen to the next most recent message
<li>Press 2 to listen to the previous message
<li>Press 3 to repeat the current message 
<li>Press 4 to listen to the most recent message
<li>Press 5 to listen to the oldest message
<li>To delete or restore a message press 7
<li>To delete or restore all messages press 9<br>
<i>
Messages will be deleted only when you hang up or go back to the main menu. <br>
In many cases we can recover accidentally deleted messages.</i>
<li>Press # to hear the date, time and phone number
<li>Press * any time to go back to the main menu<br>
<i>
Any messages you marked for deletion will be deleted at this point</i>
</ul>
</td></tr>
</table>
</body>
</html>

