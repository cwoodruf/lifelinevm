<?php
# visual user interface for recording prompts
require_once('php/globals.php');
require_once("$lib/pw/auth.php");
$ldata = login_response($_SERVER['PHP_SELF'],'ll_prompt_login');
print "<h3>Record Prompts</h3>";
$action = $_REQUEST['action'];
if ($action == 'Record') {
	require_once("prompts/english.php");
	$prompt = $_REQUEST['prompt'];
	if (!preg_match("#^ll-[\w-]+\.gsm#",$prompt)) die("invalid prompt name: $prompt");
	if ($fh = fopen("prompts/current","w")) {
		fputs($fh,"\$prompt = '$prompt';\n");
		fclose($fh);
		print <<<HTML
<i>To record this prompt dial in and enter the access code</i><br>
<h4>Prompt $prompt text:</h4>
<pre>
$prompts[$prompt]
</pre>
HTML;
		if (is_file("prompts/en/$prompt")) 
			print "<a href=\"prompts/en/$prompt\">$prompt</a><br>\n";
	} else die("couldn't save the current prompt!\n");
} else {
	foreach(glob("$asterisk_sounds/ll-*.gsm") as $pr) {
		$pr = preg_replace("#^.*/#","",$pr);
		print "<a href=\"/lifeline/prompts.php?prompt=$pr&action=Record\">$pr<br>\n";
	}
}

