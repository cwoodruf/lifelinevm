<?php
require_once('twitter.php');
header("content-type: text/plain");
print Twitter::execute('timeline');

