<?php
require("checkip.php");
header("content-type: text/plain");

$key = $_GET['key'];
if (!preg_match('#^[a-f0-9]{40}$#',$key) or !is_link($key)) return;
$msg = $_GET['msg'];
$msg = preg_replace('#\.wav$#','.gsm',$msg);
if (!preg_match('#^\d+\.\d+\.gsm$#', $msg) or !is_file("$key/$msg.txt")) return;
readfile("$key/$msg.txt");

