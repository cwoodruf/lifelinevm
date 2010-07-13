<?php 
session_start();
require_once('php/globals.php'); 
require_once("$lib/mysql.php"); 
if (isset($_SESSION['login']['login'])) $printedby = md5($_SESSION['login']['login']);

if (!ll_has_access($_SESSION['login']['vid'],ll_box($_REQUEST['box']))) {
	unset($_REQUEST);
}
?>
<html>
<head>
<title>Proof of Payment</title>
<link rel=stylesheet type=text/css href=css/base.css>
<link rel=stylesheet type=text/css href=css/admin.css>
<style type=text/css>
li {
	padding-bottom: 10px;
}
i {
	font-size: small;
}
</style>
</head>
<body>
<center>
<h3>Proof of Payment / Instructions</h3>
<table width=540 cellpadding=3 cellspacing=0 border=0>
<tr><td>
<h4>Date: <?php print date('Y-m-d'); ?></h4>
<p>
<nobr>
<b>Box:</b> <?php print $_REQUEST['box']  ? htmlentities($_REQUEST['box']) : '_______'; ?> &nbsp;&nbsp;
<b>Security code:</b> 
<?php print $_REQUEST['seccode']  ? htmlentities(ll_showcode($_REQUEST['seccode'])) : '_______'; ?>
&nbsp;&nbsp;
Payment: 
$<?php print $_REQUEST['amount']  ? sprintf('%.2f',$_REQUEST['amount']) : '_______'; ?>
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

<h4>Using voicemail</h4>
<h4>Leaving messages</h4>
<ol>
<li>Dial <b>604 248-4930</b><br> 
<li>Enter the box number to leave a message
<li>You can skip the greeting by pressing #
<li>Leave a message
<li><b>To leave another message</b> press a key and wait for the prompt
</ol>
<h4>Access your box</h4>
<ol>
<li>Dial in as above
<li>Enter the box number
<li>Enter the security code to hear the main menu:
<ul>
<li>Press 1 from the main menu to listen to messages
<li>Press 2 from the main menu to listen to your greeting
<li>Press 3 from the main menu to record your greeting
<li>Press 4 from the main menu to get your subscription date
<li>Press # from the main menu to change your security code
<li>Press * from the main menu to go back to the start<br>
<i>You can go back to the start up to 10 times in one call.</i>
</ul>
</ol>

<h4 style="page-break-before: always;">Listen to messages</h4>
<i>When you press 1 from the main menu you will hear the number of messages you have 
and your latest message followed by the listen to messages menu:</i>
<ul>
<li>Press 1 to listen to the next message
<li>Press 2 to listen to the previous message
<li>Press 3 to repeat the current message 
<li>Press 4 to listen to the newest message
<li>Press 5 to listen to the oldest message
<li>To delete or restore a message press 7
<li>To delete or restore all messages press 9<br>
<i>
Messages will be deleted only when you hang up or go back to the main menu. 
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

