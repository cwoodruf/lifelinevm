<?PHP
# purpose of this script is to present a calendar that can be used to run other scripts
# with the date as an argument
# oroginal author Frank Thurigen
require_once("globals.php");
$mn=array("0","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec");
$mnlong=array("0","January","February","March","April","May","June","July","August",
	  "September","October","November","December");

function get_calshow () {
	$dat = time();
	$dat0 = getdate($dat);
        $calshow = $_REQUEST[calshow];
        if ($calshow != "" and !preg_match("#^\w{3,20} \d{4}$#",$calshow))
                die("bad date format for $calshow!");
        if($calshow=="") $calshow=$dat0[month]." ".$dat0[year]; 
	return $calshow;
}

function get_seldate () { 
        $seldate = $_REQUEST['seldate'];
        if ($seldate != "") {
                if (!preg_match("#^\d{4}-\d{2}-\d{2}$#",$seldate))
                        die("bad date format for selected date $seldate!");
        }
	return $seldate;
}

function build_calendar ($script='',$params='',$calshow='',$align='') {
	global $mn;
	if ($calshow == '') $calshow = get_calshow();
	if ($script == '') $script = $SCRIPT_NAME;
	if ($params == '') $params = "calshow=$calshow&seldate=";
	$seldate = get_seldate();
	$dath = $calshow;
	$dat = time();
	$dat0 = getdate($dat);
	$datj = $dat0[month]." ".$dat0[year];
	$dat5 = "1 ".$dath;
	$dat1 = getdate(strtotime($dat5));
	$dat2 = getdate(strtotime($dat5." -1 month"));
	$datr = $dat2[month]." ".$dat2[year];
	$dat3 = getdate(strtotime($dat5." +1 month"));
	$datv = $dat3[month]." ".$dat3[year];
	$erster = $dat1[wday];
	# could change this to simply make a huge string 
	$cal = cal_style();
	$cal .= <<<HTML
<table class="caltab" align=$align>
<tr>
<td align=center>
HTML;
	$cal .= cal_header($datr,$dat1,$datv,$params);
	echo <<<HTML
</td>
</tr>
<tr>
<td align=center>
HTML;
	$cal .= cal_body($erster,$dat0,$dat1,$dath,$script,$params,$seldate);
	$cal .= <<<HTML
</td>
</tr>
</table>
</td>
</tr>
</table>
HTML;
	return $cal;
}
################################ calendar subs ################################
# build the top part of the calendar that lets you go to a different month
function cal_header($datr,$dat1,$datv,$params) {
	global $processurl;
	global $mn;
	$thismonth = $mn[$dat1[mon]]." ".$dat1[year];
	$iconurl = "$processurl/images/icons";
	return <<<HTML
<table class="caltab1" width=100%>
<tr>
<td align=left>
<a href="?$params&calshow=$datr" class="callink" title="go to previous month">
<IMG ALIGN="top" SRC="$iconurl/arrowleftdouble.gif" border="0" hspace="1" height="13px"></a>
</td><td align=center class="caltz">
$thismonth &nbsp;
<a href="?$params" class="callink" title="go to this month">
<IMG ALIGN="top" SRC="$iconurl/currentmonth.gif" border="0" hspace="1" height="13px" ></a>
</td><td align=right>
<a href="?$params&calshow=$datv" class="callink" title="go to next month">
<IMG ALIGN="top" SRC="$iconurl/arrowrightdouble.gif" border="0" hspace="1" height="13px"></a>
</td>
</tr>
</table>
HTML;
}
#=============================================================================#
# build the rest of the calendar for a specific month add links to the days
function cal_body ($erster,$dat0,$dat1,$dath,$script,$params,$seldate) {
	$head = <<<HTML
<table class="caltab2" width=100%>
<tr>
<td align=center class="calwt">Mon</td>
<td align=center class="calwt">Tue</td>
<td align=center class="calwt">Wed</td>
<td align=center class="calwt">Thu</td>
<td align=center class="calwt">Fri</td>
<td align=center class="calwt">Sat</td>
<td align=center class="calwt">Sun</td>
</tr>
HTML;
	if (strpos($script,'?') === false) $script .= '?';
	$script .= $params;
	if($erster!=1) {
		if($erster==0) $erster=7;
		$body .= "<tr>";
		for($i=0;$i<$erster-1;$i++) {
			$body .= "<td class=\"calat\">&nbsp;</td>\n";
		}
	}
	for($i=1; $i<=31; $i++) {
		$dat4 = getdate(strtotime($i." ".$dath));
		$heute = $dat4[wday];
		if($heute==0) $heute=7;
		if($dat1[mon]==$dat4[mon]) {
			$stil="calat";
			$dum1=$dat0[mday].".".$dat0[mon].".".$dat0[year];
			$dum2=$dat4[mday].".".$dat4[mon].".".$dat4[year];
			if($dum1==$dum2) {
				if(($heute==6) || ($heute==7)) {
					$stil="calhe"; 
					$stil_l="sellinkbold"; 
				} else {
					$stil="calht"; $stil_l="sellink"; 
				}
			} else{
				if(($heute==6) || ($heute==7)) {
					$stil="calwe"; 
					$stil_l="sellinkbold"; 
				} else {
					$stil="calat"; 
					$stil_l="sellink"; 
				}
			}
			$i = sprintf('%02d',$i);
			if($heute==1) $body .= "<tr>";
			$seldate = sprintf("%04d-%02d-%02d",$dat1[year],$dat1[mon],$i);
			$body .= "<td align=center class=\"".$stil."\">".
				 "<a class=$stil_l href=\"${script}$seldate\">".$i.
				 "</a></td>\n";
			if($heute==7) $body .= "</tr>";
			$j = $heute;
		}
	}
	if ($j < 7) {
		for ($i=0;$i<7-$j;$i++) {
			$body .= "<td class=\"calat\">&nbsp;</td>\n";
		}
		$body .= "</tr>\n";
	}
	return $head.$body;
}
#=============================================================================#
# print the main style block for the calendar this includes its position
function cal_style () {
	$font_family = "font-family: verdana,arial,helvetica,sans-serif;";
	$font_size = "font-size: xx-small;";
	$textd = "text-decoration:none;";
	$standard = "$font_family $font_size $textd";
	$bold = "font-weight: bold; $standard";
	return <<<CSS
<style type="text/css">
<!--
a.sellinkbold:link 	{ color:#FF0000; $standard }
a.sellinkbold:visited 	{ color:#FF00FF; $standard }
a.sellinkbold:active 	{ color:#FF00FF; $standard }
a.sellinkbold:hover 	{ color:#0000FF; $standard }
a.sellink:link 		{ color:#000000; $standard }
a.sellink:visited 	{ color:#0000FF; $standard }
a.sellink:active 	{ color:#0000FF; $standard }
a.sellink:hover 	{ color:#FF0000; $standard }
a.callink:link 		{ color:#000000; $bold }
a.callink:visited 	{ color:#000000; $bold }
a.callink:active 	{ color:#000000; $bold }
a.callink:hover 	{ color:#000000; $bold }
a.calhome:link 		{ color:#000000; $standard }
a.calhome:visited 	{ color:#000000; $standard }
a.calhome:active 	{ color:#000000; $standard }
a.calhome:hover 	{ color:#000000; $standard }
td.caltz 		{ color:#000000; $font_family $font_size }
td.calwt 		{ color:#000000; $font_family $font_size font-weight: bold; }
td.calat 		{ color:#000000; $font_family $font_size }
td.calht 		{ color:#000000; background-color:#AAAAFF; $font_family $font_size }
td.calhe 		{ color:#FF0000; background-color:#AAAAFF; $font_family $font_size }
td.calwe 		{ color:#FF0000; $font_family $font_size }
table.caltab1 		{ border:0px; padding:0px; margin:0px; }
table.caltab2 		{ border:0px; padding:0px; margin:0px; }
table.caltab { 
	width:160px; 
	border:1px solid grey; 
	background-color:#FFFFFF; 
	padding:2px; 
	margin:0px; 
}
//-->
</style>
CSS;
}


