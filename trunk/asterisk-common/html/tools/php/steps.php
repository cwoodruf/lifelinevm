<?php

$steps = array("import","slot","process","release");

function get_step () {
	global $steps;
	$step = $_POST[step] or $step = $_GET[step];
	$step_pat = "#^(".implode('|',$steps)."|)$#";
	if (!preg_match($step_pat,$step)) die("bad step name $step!");
	return $step;
}

function step_select($step = "import") {
	global $steps;
	$radios = array();
	foreach ($steps as $s) {
		$html = "<td><input name=step type=radio value=\"$s\" ";
		if ($step == $s) $html .= "checked";
		$html .= "></td><td><b>$s<b></td>";
		array_push($radios,$html);
	}
	return "<table><tr valign=middle>".implode("<td>===&gt;</td>",$radios)."</tr></table>";
}

