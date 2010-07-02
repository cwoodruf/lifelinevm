<?php 
session_start();
require_once('php/globals.php'); 
require_once("$lib/mysql.php"); 
if (!ll_has_access($_SESSION['login']['vid'],ll_box($_REQUEST['box']))) {
	unset($_REQUEST);
}
?>
<html>
<head>
<title>Proof of Payment</title>
<link rel=stylesheet type=text/css href=css/base.css>
<link rel=stylesheet type=text/css href=css/admin.css>
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
Payment: $_______________
</nobr>
<p>
<div style="padding-top: 30px;">
Received by: __________________________________________
</div>
<i style="font-size: small;">
This is your receipt and proof of purchase.<br>
Please keep this receipt in case of problems with your account.<br>
</i>

<h4>Using voicemail</h4>
<h4>Leaving messages</h4>
<ol>
<li>Dial <b>604 248-5760</b><br> 
<li>Enter the box number to leave a message
<li>You can skip the greeting by pressing #
<li>Leave a message
<li>If you want to <b>leave another message</b> press a key and wait for the prompt
</ol>
<h4>Listening to messages</h4>
<h4>By phone</h4>
<ol>
<li>Dial in as above
<li>Enter the box number
<li>Enter the security code 
<li>Press 1 from the main menu to listen to messages
<li>Use the next menu (listen to messages menu) to navigate around the messages
<li>Press * to go back to the start
</ol>
</td></tr>
</table>
</body>
</html>

