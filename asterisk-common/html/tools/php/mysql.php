<?php # mysql related php "middleware"
# $Id: mysql.php,v 1.14 2008/10/26 04:59:15 root Exp root $
if (defined(DBLOGINFILE)) @include(DBLOGINFILE); 
eval(file_get_contents($SALTFILE)); 

###############################################################################
# lifeline section
function get_salestax($idate) {
	if ($idate < '2010-07-01') {
		return array( 'rate' => 0.05, 'name' => 'GST' );
	} else if ($idate >= '2011-08-08') {
		return array( 'rate' => 0, 'name' => 'HST' );
	}
	return array( 'rate' => 0.12, 'name' => 'HST' );
}
$net_due = 30; # grace period for paying invoices in days
$min_months = 4; # minimum # months you can buy
# personal information associated with a box
if (!isset($personal_fields)) {
	$personal_fields = array(
		'name' => 'Name',
		'email' => 'Email',
		'notes' => 'Notes',
		'llphone' => 'Phone',
		'paidto' => 'Paid to',
		'cid' => 'Client ID',
	);
}

function squashedphone($phone) {
	$squashedphone = preg_replace('#.*?((?:\d\D*){10}).*#',"$1",$phone);
	$squashedphone = preg_replace('#\D#','',$squashedphone);
	return $squashedphone;
}

function ll_connect () {
	global $lldb, $ll_login, $ll_password, $ll_host, $ll_port, $ll_dbname;
	# if (isset($lldb)) return $lldb;
	if (empty($ll_host)) $ll_host = 'localhost';
	if (empty($ll_dbname)) $ll_dbname = 'lifeline';
	if (preg_match('#^\d+$#',$ll_port)) $port = ";port=$ll_port";
	try {
		$dsn = "mysql:dbname=$ll_dbname;host=$ll_host$port";
		$lldb = new PDO($dsn, $ll_login,$ll_password);
	} catch (Exception $e) {
		# if the login fails try localhost
		try {
			print "connect error: using defaults!<br>\n";
			$lldb = new PDO(
				"mysql:dbname=$ll_dbname",
				$ll_login,$ll_password
			);
		} catch (Exception $e) {
			die("connect failed: ".$e->getMessage());
		}
	}
	return $lldb;
}

function ll_disconnect () {
	// there does not seem to be any way to close a db handle with PDO
}

function ll_vendor_ok($vendor) {
	return preg_match('#^[ \w\(\)\[\]\-]{1,128}#',$vendor);
}

function ll_vendor($vid,$refresh=true) {
	global $vend;
	if (!preg_match('#^\d+$#',$vid)) return;
	if (isset($vend) and $vend['vid'] === $vid and !$refresh) return $vend;
	$vend = ll_load_from_table('vendors','vid',$vid,false,'',$refresh);
	if ($vend === false) return false;
	$vend['unpaid_months'] = ll_get_unpaid_months($vid);
	return $vend;
}

function ll_vendor_from_email($email) {
	$lldb = ll_connect();
	$query = "select vendors.* from vendors where email like ".$lldb->quote("%$email%");
	$st = $lldb->query($query);
	if ($st === 'false') die(ll_err());
	$row = $st->fetch(PDO::FETCH_ASSOC);
	if (!is_array($row)) return false;
	foreach ($row as $name => $value) {
		$data[$name] = $value;
	}
	return $data;
}

function ll_vendor_from_id($id) {
	$lldb = ll_connect();
	$query = "select vendors.vendor,vendors.vid,emailsignup.email,emailsignup.perms ".
		"from vendors join emailsignup on (vendors.vid=emailsignup.vid) ".
		"where emailsignup.id=".$lldb->quote($id);
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$row = $st->fetch(PDO::FETCH_ASSOC);
	if (!is_array($row)) return false;
	foreach ($row as $name => $value) {
		$data[$name] = $value;
	}
	return $data;
}

function ll_undelete_vendor(&$vdata) {
	$vid = $vdata['vid'];
	if (!preg_match('#^\d+$#',$vid)) die("undelete_vendor: vid is not a number!");
	$vdata['status'] = $vend['status'] = '';
	ll_save_to_table('update','vendors',$vend,'vid',$vid);
}

function ll_emailsignup_id($email, $vendor,$perms='boxes') {
	$lldb = ll_connect();

	$id = md5(mt_rand().$email);
	$affected = $lldb->exec(
		"replace into emailsignup (vid,email,perms,id) values (".
		$lldb->quote($vendor['vid']).','.
		$lldb->quote($email).','.
		$lldb->quote($perms).','.
		$lldb->quote($id).
		")"
	);
	if ($affected === false) die(ll_err());
	return $id;
}

function ll_del_vendor($vid) {
	$lldb = ll_connect();
	if (!preg_match('#^\d+$#',$vid) or $vid <= 0) die("del_vendor: bad vendor id $vid!");
	$result = $lldb->exec(
		"update vendors set status='deleted' where vid='$vid'");
	if ($result === false) die("del_vendor: ".$lldb->errInfo());
	return true;
}

function ll_get_unpaid_months($vid) {
	$invoices = ll_load_from_table('invoices','vid',$vid);
	foreach ($invoices as $idata) {
		if (preg_match('#^(0000-00-00 00:00:00|)$#',$idata['paidon'])) continue;
		$months += $idata['months'];
	}
	return $months;
}

function ll_vendors($vid) {
	$lldb = ll_connect();
	if (!preg_match('#^\d*#',$vid)) die("invalid vendor id!");
	if ($vid > 0) {
		$pat = ll_parentpat($vid);
		$getvendors = " and (vid='$vid' or parent regexp '$pat') ";
	}
	$query = "select * from vendors where status not in ('deleted') $getvendors order by vendor";
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$rows = $st->fetchAll();
	if (count($rows)) return $rows;
	return false;
}

function ll_add_user($vend,$oldlogin,$login,$password,$perms,$notes='') {
	if (!preg_match('#^\d+$#',$vend['vid'])) 
		die("Bad vendor ".$vend['vid'].", ".$vend['vendor']."!");
	if (!preg_match('#^\S{1,64}$#',$login)) die("bad login $login!");
	if (!preg_match('#^\S{0,64}$#',$oldlogin)) die("bad oldlogin $oldlogin!");
	if (!preg_match('#^[\w:]*$#',$perms)) die("bad perms $perms!");
	if (!empty($password)) {
		if (!preg_match('#^\S{1,32}$#',$password)) die("bad password $password!");
		$udata['password'] = md5($password);
	}
	$lldb = ll_connect();
	$udata['vid'] = $vend['vid'];
	$udata['login'] = $login;
	if ($perms) $udata['perms'] = $perms;
	$udata['created'] = date('Y-m-d H:i:s');
	$udata['notes'] = $notes;
	if ($oldlogin and $olduser = ll_user($oldlogin)) {
		return ll_save_to_table('update','users',$udata,'login',$oldlogin);
	}
	if (!$udata['password']) die("need a password to insert a new user record!");
	return ll_save_to_table('replace','users',$udata,null,$uid);
}

function ll_user($login,$refresh=true) {
	return ll_load_from_table('users','login',$login,false,'',$refresh);
}

function ll_del_user($vend,$login) {
	global $lldb;
	if (!isset($vend)) die("no vendor!");
	if (!preg_match('#^\S{1,64}$#',$login)) die("bad login $login!");
	$udata = ll_pw_data($login);
	// user doesn't exist
	if ($udata['vid'] === null) return;
	$vid = $vend['vid'];
	if ($udata['vid'] != $vid) 
		die("user $login does not belong with vendor ".$vend['vendor']." ($vid)"); 
	$login = $lldb->quote($login);
	$rows = $lldb->exec("delete from users where login=$login and vid=$vid");
	if ($rows === false) die(ll_err());
}
	
function ll_users($vend) {
	return ll_load_from_table('users','vid',$vend['vid']);
}

function ll_check_box($box,$die=true) {
	if (preg_match('#^\d{4,32}$#',$box)) return true;
	else if ($die) die("bad box number '$box'!");
	return false;
} 

function ll_check_seccode($seccode) {
	if (!preg_match('#^\d{4}$#',$seccode)) die("Invalid security code $seccode!");
	return true;
}

function ll_paycodeinfo($paycode) {
	if (!preg_match('#^(\d{4}) *(\d{4}) *(\d{4})#',$paycode,$m)) return;
	$code = $m[1].' '.$m[2].' '.$m[3];
	$query = "select * from paycode where code like '$code%'";
	$lldb = ll_connect();
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$row = $st->fetch();
	return $row;
}

function ll_box($box,$refresh=true) {
	if (ll_check_box($box,($die=false))) return ll_load_from_table('ourboxes','box',$box,false,'',$refresh);
}

function ll_pobox($box,$refresh=true) {
	return ll_load_from_table('poboxes','box',$box,false,'',$refresh);
}

function ll_pobox_ok($box) {
	if (!preg_match('#^\d+$#',$box)) return;
	if ($box < 1) return;
	if ($box > 90) return;
	return $box;
}

function ll_clients_pobox($cid,$refresh=true) {
	return ll_load_from_table('poboxes','cid',$cid,false,'',$refresh);
}

function ll_client_from_name($name, $refresh=true) {
	return ll_load_from_table('clients','name',$name,false,'',$refresh);
}

function ll_client($cid, $refresh=true) {
	return ll_load_from_table('clients','cid',$cid,false,'',$refresh);
}

function ll_client_insert($vend, $cdata) {
	return ll_client_modify($vend, $cdata, 'insert');
}

function ll_client_update($vend, $cdata) {
	return ll_client_modify($vend, $cdata, 'update');
}

function ll_client_modify($vend, $cdata, $op) {
	global $ldata, $personal_fields;
	unset($personal_fields['llphone']);
	unset($personal_fields['paidto']);

	foreach ($cdata as $field => $value) {
		if (!isset($personal_fields[$field])) unset($cdata[$field]);
	}
	$cdata['login'] = $ldata['login'];
	$cdata['app'] = $ldata['app'];
	$cdata['vid'] = $vend['vid'];
	$cid = $cdata['cid'];
	if (ll_save_to_table($op,'clients',$cdata,'cid',$cid,true)) 
		return $cid;
	return false;
}

function ll_clients_poboxes($cid,$refresh=true) {
	return ll_load_from_table('poboxes','cid',$cid,true,'',$refresh);
}

function ll_calls($box,$limit=null) {
	if (preg_match('#^\d+$#',$limit)) $limitreq = "limit $limit";
	if (ll_check_box($box,($die=false))) 
			return ll_load_from_table('calls','box',$box,true,"order by from_unixtime(callstart) desc $limitreq ");
}

function ll_calls_by_date($vid,$date) {
	$lldb = ll_connect();
	if (!preg_match('#^\d+$#', $vid)) die("calls_by_date: bad vendor id $vid!");
	if (!preg_match('#^\d{4}-\d{2}-\d{2}$#', $date)) die("calls_by_date: bad date $date!");
	$pat = ll_parentpat($vid);
/*
# (not so) simple query which won't pick up the ll-callstart.pl actions as we don't know the box number at that point
	$query = "select calls.*,vendors.vendor,vendors.vid ".
		"from calls left outer join boxes on (calls.box=boxes.box) join vendors on (vendors.vid=boxes.vid) ".
		"where (vendors.vid='$vid' or vendors.parent regexp '$pat') ".
		"and from_unixtime(callstart) between '$date 00:00:00' and '$date 23:59:59' ".
		"order by from_unixtime(callstart) desc, call_time desc, action";
*/
	# more complex query that can pick up actions with no box number or vid associated
	$query = "select calls.*,vendors.vendor,vendors.vid ".
		"from calls join boxes on (calls.box=boxes.box) ".
		"join vendors on (vendors.vid=boxes.vid) ".
		"where (vendors.vid='$vid' or vendors.parent regexp '$pat') ".
		"and from_unixtime(callstart) between '$date 00:00:00' and '$date 23:59:59' ";
	# this union picks up the ll-callstart.pl actions where we have no box number
	if ($vid == ROOTVID) $query .=
		"union select calls.*,'',0 from calls ".
		"where from_unixtime(callstart) between '$date 00:00:00' and '$date 23:59:59' ".
		"and calls.box = '' ";
	$query .= 
		"order by from_unixtime(callstart) desc, call_time desc, action";
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$calls = array();
	while ($row = $st->fetch()) {
		$calls[] = $row;
	}
	return $calls;
}

function ll_payments_by_date($vid,$date) {
	$lldb = ll_connect();
	if (!preg_match('#^\d+$#', $vid)) die("payments_by_date: bad vendor id $vid!");
	if (!preg_match('#^\d{4}-\d{2}-\d{2}$#', $date)) die("payments_by_date: bad date $date!");
	$pat = ll_parentpat($vid);
	$query = "select payments.*,vendors.vendor,vendors.vid ".
		"from payments join boxes on (payments.box=boxes.box) join vendors on (vendors.vid=boxes.vid) ".
		"where (vendors.vid='$vid' or vendors.parent regexp '$pat') ".
		"and paidon between '$date 00:00:00' and '$date 23:59:59' ".
		"order by paidon desc";
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$payments = array();
	while ($row = $st->fetch()) {
		$payments[] = $row;
	}
	return $payments;
}

function ll_calldates($vid) {
	$lldb = ll_connect();
	if (!preg_match('#^\d+$#', $vid)) die("calldates: bad vendor id $vid!");
	$pat = ll_parentpat($vid);
	$query = "select distinct date(from_unixtime(callstart)) as day ".
		"from calls join boxes on (calls.box=boxes.box) join vendors on (vendors.vid=boxes.vid) ".
		"where (vendors.vid='$vid' or vendors.parent regexp '$pat') ".
		"order by day desc ";
	$st = $lldb->query($query);
	If ($st === false) die(ll_err());
	$dates = array();
	while ($row = $st->fetch()) {
		$dates[] = $row['day'];
	}
	return $dates;
}

function ll_paymentdates($vid) {
	$lldb = ll_connect();
	if (!preg_match('#^\d+$#', $vid)) die("calldates: bad vendor id $vid!");
	$pat = ll_parentpat($vid);
	$query = "select distinct date(paidon) as day ".
		"from payments join boxes on (payments.box=boxes.box) join vendors on (vendors.vid=boxes.vid) ".
		"where (vendors.vid='$vid' or vendors.parent regexp '$pat') ".
		"order by day desc ";
	$st = $lldb->query($query);
	If ($st === false) die(ll_err());
	$dates = array();
	while ($row = $st->fetch()) {
		$dates[] = $row['day'];
	}
	return $dates;
}

function ll_boxcount($vend,$showkids=false,$refresh=true) {
	static $boxcounts;
	if (is_array($vend)) $vid = $vend['vid'];
	else $vid = $vend;
	if (!$refresh and $boxcounts[$vid]) return $boxcounts[$vid];
	$pat = ll_parentpat($vid);
	if ($showkids) 
		$what = "(boxes.vid='$vid' or vendors.parent regexp '$pat')";
	else 
		$what = "boxes.vid='$vid'";
	$query = "select count(*) as howmany from boxes join vendors on (boxes.vid=vendors.vid) ".
		"where $what ". 
		"and boxes.status <> 'deleted' and (paidto >= current_date() or paidto = 0)";
	$lldb = ll_connect();
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$row = $st->fetch();

	$boxcounts[$vid] = $row[0];
	return $row[0];
}

function ll_boxes($vend,$showkids=false,$status='not_deleted',$order = "order by paidto desc") {
	if ($status == 'deleted') 
		$status = " and (boxes.status in ('deleted') or paidto < current_date()) ";
	else if ($status == 'not_deleted') 
		$status = " and boxes.status not in ('deleted') and (paidto >= current_date() or paidto = 0) ";

	if (is_array($vend)) $vid = $vend['vid'];
	else $vid = $vend;
	
	if ($showkids) {
		$pat = ll_parentpat($vid);
		$query = "select boxes.*,vendors.vendor from boxes join vendors on (boxes.vid=vendors.vid) ".
			"where (boxes.vid=$vid or vendors.parent regexp '$pat') ".
			"$status $order";
		$lldb = ll_connect();
		$st = $lldb->query($query);
		if ($st === false) die(ll_err());
		return $st->fetchAll();
	}
	return ll_load_from_table('boxes','vid',$vid,true," $status $order");
}

function ll_poboxes($status='all',$date=null) {
	# force a numeric sort by box
	$order = " order by box+0";
	$where = "";

	if ($status == 'deleted_first')  { 
		$order = " order by paidto, box+0";
	} else if ($status == 'recent_first') {
		if (preg_match('#^\d\d\d\d-\d\d-\d\d$#', $date)) 
			$where = " where paidto <= '$date' ";
		$order = " order by box+0";
	}

	return ll_load_from_table('poboxes',null,null,true," $where $order");
}

function ll_activeboxcount($vid) {
	if (!preg_match('#^\d+$#',$vid)) die("bad vid in ll_activeboxcount!");
	return ll_boxcount($vid,($showkids=false));
}

function ll_logincount($vid) {
	if (!preg_match('#^\d+$#',$vid)) die("bad vid in ll_logincount!");
	$query = "select count(*) from users where vid='$vid'";
	$lldb = ll_connect();
	$st = $lldb->query($query);
	if ($st == false) die(ll_err());
	$row = $st->fetch();
	return $row[0];
}

function ll_find_box($vend,$box,$seccode) {
	global $salt, $SALTFILE;
	eval(file_get_contents($SALTFILE)); 
	$lldb = ll_connect();
	if (!preg_match('#^\d+$#',$box)) die("box should be a number!");
	$where = " where box=$box and seccode='".md5($seccode.$salt)."'";
	return ll_load_from_table('ourboxes',null,null,true,$where);
}

function ll_find_boxes($vend,$search) {
	$lldb = ll_connect();
	# $where = " and status <> 'deleted' and (";
	$pat = ll_parentpat($vend);
	$where = " where (vid='$vend' or parent regexp '$pat') and (";
	if (empty($search)) {
		$where .= "1=1";
	} else {
		if (preg_match('#^\s*-\s*(.*)#',$search,$m)) {
			$search = $m[1];
			$not = 'not';
			$andor = 'and';
		} else {
			$not = '';
			$andor = 'or';
		}
		# foreach (array('box', 'name', 'email', 'paidto', 'notes', 'status', 'login') as $field) {
		foreach (array('box', 'name', 'email', 'paidto', 'vendor', 'status') as $field) {
			$value = $lldb->quote($search);
			$wheres[] = "$field $not regexp ($value)";
		}
		$where .= implode(" $andor ",$wheres);
	}
	$where .= ") order by box";
	return ll_load_from_table('ourboxes',null,null,true,$where);
}

function ll_find_clients($vend,$search) {
	$lldb = ll_connect();
	$where = " where (";
	if (empty($search)) {
		$where .= "1=1";
	} else {
		if (preg_match('#^\s*-\s*(.*)#',$search,$m)) {
			$search = $m[1];
			$not = 'not';
			$andor = 'and';
		} else {
			$not = '';
			$andor = 'or';
		}
		if (preg_match('#pobox\s*(\d+)#i', $search, $m)) {
			$value = $lldb->quote($m[1]);
			if ($not) {
				$wheres[] = "pobox <> $value";
			} else {
				$wheres[] = "pobox = $value";
			}
			$search = preg_replace('#pobox\s*(\d+)#i','',$search);
		} else {
			if (preg_match('#pobox#', $search)) {
				$search = str_replace('pobox','',$search);
				$where .= "pobox is not null) and (";
			}
		}
		if (preg_match('#\S#',$search)) {
			foreach (array('pobox','box','client_name','name','pobox_name','status',
					'email','pobox_email','paidto','pobox_paidto') as $field) {
				$value = $lldb->quote(trim($search));
				$wheres[] = "$field $not regexp ($value)";
			}
		}
		$where .= implode(" $andor ",$wheres);
	}
	$where .= ") order by pobox+0,box+0";
	return ll_load_from_table('ourclients',null,null,true,$where);
}

function ll_check_boxlimit($vend,$dummy=null) {
	$boxes = ll_boxcount($vend);
	if ($boxes < $vend['box_limit']) return true;
	die("vendor {$vend['vendor']} has met or exceeded their box limit {$vend['box_limit']}");
}

function ll_check_months($vend,$months) {
	if (!preg_match('#^-?\d+$#',$months))
		die("Invalid months value $months!");
	if (is_numeric(MAXMONTHS) and abs($months) > MAXMONTHS) 
		die("Months value $months exceeds maximum ".MAXMONTHS."!");
	if (!isset($vend['months'])) {
		if (!is_array($vend)) {
			if (!preg_match('#^\d+$#',$vend)) die("check_months: bad vendor id!");
			$vend = ll_vendor($vend);
		} else if (!preg_match('#^\d+$#',$vend['vid'])) {
			die("check_months: bad vendor id from array!");
		} else {
			$vend = ll_vendor($vend['vid']);
		}
	}
	if ($vend['months'] < $months) 
		die("Vendor ".$vend['vendor']." only has ".$vend['months']." month(s) available!");
	return $months;
}

function ll_box_months_left($bdata) {
	if (!is_array($bdata)) $bdata = ll_box($bdata);
	$m = ll_months_left($bdata['paidto']); 
	$n = ll_add_months($bdata['status']);
	return $m + $n;
}

function ll_box_add_months($bdata) {
	if (!is_array($bdata)) $bdata = ll_box($bdata);
	return ll_add_months($bdata['status']);
}

function ll_add_months($status) {
	if (preg_match('#add (\d+) months#', $status, $m)) 
		return $m[1];
	return 0;
}

function ll_months_left($date) {
	# crude but generous accounting for a month ...
        $months = round((strtotime($date) - time())/(28*86400));
        if ($months < 0) $months = 0;
        return $months;
}

function ll_get_owing($vend=null) {
	$lldb = ll_connect();
	$query = "select vid,sum(total) as owed from invoices where (paidon is null or paidon = 0) ";
	if (isset($vend) and preg_match('#^\d+$#',$vend['vid'])) 
		$query .= "and vid='".$vend['vid']."' ";
	$query .= "group by vid";
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	if (isset($vend)) {
		$row = $st->fetch();
		return $row[1];
	}
	while ($row = $st->fetch()) {
		$owing[$row[0]] += $row[1];
	}
	return $owing;
}

# switch vendors for a box we assume you've checked the sanity of this prior
function ll_transfer($what,$value,$currvendor,$newvendor) {
	$lldb = ll_connect();
	switch($what) {
	case 'box': 
		$query = sprintf("update boxes set vid=%u where box=%u and vid=%u",
				$newvendor['vid'], $value, $currvendor['vid']);
		$affected = $lldb->exec($query);
		if ($affected) return true;
		return false;

	case 'months': 
		$query = sprintf("update vendors set months=months-%u where vid=%u",
				$value, $currvendor['vid']);
		$affected = $lldb->exec($query);
		if (!$affected) die("couldn't deduct $months months from {$currvendor['vid']}!");

		$query = sprintf("update vendors set months=months+%u where vid=%u",
				$value, $newvendor['vid']);
		$affected = $lldb->exec($query);
		if (!$affected) die("couldn't add $months months to {$newvendor['vid']}!");
		# log the transfer
		return true;

	default: die("not sure what I'm supposed be doing?");
	}
}

function ll_logmonthstransfer($fromvid,$tovid,$months,$login) {
	$lldb = ll_connect();
	$query = sprintf(
		"insert into monthstransfer (transferred,fromvid,tovid,months,login) ".
		"values (now(), %u, %u, %d, %s)",
		$fromvid, $tovid, $months, $lldb->quote($login)
	);
	$affected = $lldb->exec($query);
	if ($affected) return true;
	return false;
}

function ll_get_invoiced($vend=null) {
	$lldb = ll_connect();
	$query = "select vid,sum(total) from invoices ";
	if (isset($vend) and preg_match('#^\d+$#',$vend['vid'])) 
		$query .= "where vid='".$vend['vid']."' ";
	$query .= "group by vid";
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	if (isset($vend)) {
		$row = $st->fetch();
		return $row[1];
	}
	while ($row = $st->fetch()) {
		$invoiced[$row[0]] = $row[1];
	}
	return $invoiced;
}

# audit trail type of log - can also be used with paycodes if they get implemented
function ll_log($vend,$cdata) {
	global $ldata;
	$cdata['login'] = $ldata['login'];
	$cdata['app'] = $ldata['app'];
	$cdata['vid'] = $vend['vid'];
	return ll_save_to_table('insert','updates',$cdata,null,$vid,true);
}

# find a new box based on a range of numbers
# by default do not put in a subscription date - this is done when the box is called for the first time
function ll_new_box($trans,$vend,$months,$llphone,$min_box,$max_box,$creditcheck='ll_check_months') {
	global $ldata, $salt;

	$vmnewboxsem = sem_get(6823269);
	if ($vmnewboxsem) sem_acquire($vmnewboxsem);
	else print "ERROR: can't get semaphore $vmnewboxsem for ll_new_box<br>\n";

	if (!preg_match('#^\d+$#',$min_box) or !preg_match('#^\d+$#',$max_box)) {
		die("new_box: bad box range $min_box, $max_box!");
	}
	if ($min_box > $max_box) list($max_box,$min_box) = array($min_box,$max_box);

	$cutoff_interval = '6 months';
	if (function_exists($creditcheck)) {
		$creditcheck($vend,$months);
	} else {
		die("no / invalid credit check function!");
	}
	$vdata['months'] = $vend['months'] - $months;
	$lldb = ll_connect();

	# we can activate specific boxes only if they are not in use
	if ($max_box == $min_box) {
		$box = $min_box;
		if (!ll_is_available(ll_box($box))) die("box $box currently in use!");
	} else {
		$box = ll_find_available_box($min_box,$max_box);
	}

	$seccode = sprintf('%04d',rand(0,9999));
	if ($months == 0) {
		$add = "permanent";
	} else {
		$add = "add $months months";
	}

	$qllphone = $lldb->quote($llphone);
	$query = "replace into boxes (box,seccode,vid,llphone,".
		        "paidto,login,modified,created,status,trans,canremove,cid) ".
		"values ('$box',md5('$seccode$salt'),'$vend[vid]',$qllphone,".
		        "'$paidto','$ldata[login]',now(),now(),'$add','$trans','$months',null)";

	$result = $lldb->exec($query);
	if ($result === false) die(ll_err());

	ll_log($vend,array('newpaidto'=>$paidto,'box'=>$box,'months'=>$months,'action'=>'new_box'));
	if (ll_save_to_table('update','vendors',$vdata,'vid',$vend['vid'])) 
		return array($box,$seccode,$paidto);

	if ($vmnewboxsem) sem_release($vmnewboxsem);
}

function ll_is_available($bdata) {
	if (empty($bdata) or $bdata['status'] == 'deleted' )
	{
		return true;
	}
	return false;
}

function ll_find_available_box($min_box,$max_box) {
	$lldb = ll_connect();
	# figure out what boxes are available (save this in case we need to do more than one)
	if (!is_array($available)) {
		$available = array();
		$st = $lldb->query(
				"select box,paidto,status from boxes ".
				"order by box"
			);
		if ($st === false) die(ll_err());
		while ($row = $st->fetch()) {
			list($box,$paidto,$status) = $row;
			$available[$box] = ll_is_available($row);
		}
		$st->closeCursor();
		for ($i = MINBOX; $i <= MAXBOX; $i++) {
			if (isset($available[$i])) continue;
			$available[$i] = true;
		}
	}
	# now pick an unused box
	for ($i=$min_box; $i<=$max_box; $i++) {
		if ($available[$i]) {
			return $i;
			$available[$i] = false;
			break;
		}
	}
}

function ll_set_paidto($box,$startdate) {
	$vmnewboxsem = sem_get(6823269);
	if ($vmnewboxsem) sem_acquire($vmnewboxsem);
	else print "ERROR: can't get semaphore $vmnewboxsem for ll_set_paidto<br>\n";

	$bdata = ll_box($box);
	if (!is_array($bdata)) die("ll_set_paidto: no data for box $box!");
	if ($bdata['paidto'] > 0) die("this box has already been claimed by {$bdata['login']}!");
	if (!preg_match('#^\d\d\d\d-\d\d-\d\d$#',$startdate)) die("ll_set_paidto: invalid start date!");
	if (preg_match('#add (\d+) month#', $bdata['status'], $m)) {
		$months = $m[1];
		$timestr = "$startdate +$months months";
		$paidto = date('Y-m-d',strtotime($timestr));
		if (preg_match('#^2\d\d\d-\d\d-\d\d$#', $paidto)) {
			$update['status'] = '';
			$update['paidto'] = $paidto;
			ll_save_to_table('update','boxes',$update,'box',$box);
		}
	}

	if ($vmnewboxsem) sem_release($vmnewboxsem);
}

function ll_showcode($seccode) {
	global $salt;

	# if its not md5-like then return it
	if (!preg_match('#^\w{32,}$#',$seccode)) return $seccode;

	# as this is somewhat expensive maybe discourage overuse
	# sleep(3);

	$lldb = ll_connect();
	$st = $lldb->query(
		"select i,md5(concat(lpad(i,4,'0'),'$salt')) as seccode ".
		"from numrange where i between 0 and 9999 order by i"
	);
	if ($st === false) die(ll_err());
	while ($row = $st->fetch(PDO::FETCH_ASSOC)) {
		$i = $row['i'];
		if ($row['seccode'] == $seccode) return str_pad($i, 4, "0", STR_PAD_LEFT);
	}
	# for anything we don't know about in table fill it in
	for (; $i<=9999; $i++) {
		$lldb->exec("insert into numrange (i) values ($i)");
	}
	return false;
}

function ll_add_time($vend,$box,$months) {
	$udata = ll_check_time($vend,$box,$months);
	if (!is_array($vend)) $vend = ll_vendor($vend);
	if (ll_save_to_table('update','boxes',$udata,'box',$box)) {
		ll_log($vend, array('oldpaidto'=>$bdata['paidto'],'newpaidto'=>$udata['paidto'],
				'box'=>$box,'months'=>$months,'action'=>'add_time','login'=>$ldata['login']));
		$vdata['months'] = $vend['months'] - $months;
		if (ll_save_to_table('update','vendors',$vdata,'vid',$vend['vid'])) 
			return $udata['paidto'];
	}
}

function ll_check_time($vend,$box,$months) {
	global $ldata;
	global $pt_cutoff;
	if (!is_array($vend)) $vend = ll_vendor($vend);

	if (!preg_match('#^\d+$#',$vend['vid'])) die("check_time: bad vendor id!");
	if (!preg_match('#^\d+$#',$box)) die("check_time: bad box id!");
	if (!preg_match('#^-?\d\d?$#',$months)) die("invalid months value!");

	ll_check_box($box);
	$bdata = ll_box($box);
	if ($months > 0) {
		ll_check_months($vend,$months);
	# this code needs to be refactored as vendors can have sub accounts where this logic
	# would likely not apply
	# } else {
	#	if (abs($months) > $bdata['canremove']) 
	#		die("Error: you can only remove {$bdata['canremove']} months!");
	}

	# if the box exists but has never been used
	if (($addmonths = ll_box_add_months($bdata)) > 0) {
		$newmonths = $addmonths + $months;
		# this can be zero or less as we can delete months from it
		if ($newmonths > 0) $udata['status'] = "add $newmonths months";
		else $udata['status'] = "deleted";

	# if the box has no paidto date at all
	} else if ($bdata['paidto'] == 0) {
		if ($months < 0) die("invalid number of months: should be zero or more!");
		# $udata['paidto'] = date('Y-m-d',strtotime(date('Y-m-d')." +$months months"));
		$udata['status'] = "add $months months";

	# if the box has time
	} else {
		$op = preg_match('#^-#',$months) ? '' : '+';
		# if you are subtracting time make sure box has enough time to subtract from
		if ($op == '' and abs($months) > ll_box_months_left($bdata))
			die("Box $box does not have ".abs($months)." months left!");

		$paidto = strtotime($bdata['paidto']);
		if ($paidto < time() - $pt_cutoff) { $start = date('Y-m-d'); }
		else $start = $bdata['paidto'];

		$udata['paidto'] = date('Y-m-d',strtotime("$start $op$months months"));
		$udata['status'] = '';
	}

	$udata['login'] = $ldata['login'];
	$udata['vid'] = $vend['vid'];

	# canremove determines how many months you can realistically delete
	# this is only really significant if the box has moved to another vendor 
	# with time on it from the previous vendor basically its a high water mark
	# this logic is muddied at this point because vendors can have sub vendors
	# in most cases a customer uses all of the time plus some so its unlikely 
	# months would get subtracted from a box without the customer knowing
	# nevertheless it is possible and should probably still be checked somehow
	# the checking code is commented out currently
	if (!ll_isparent($vend['vid'],$bdata)) {
		$udata['canremove'] = $months;
	} else {
		$udata['canremove'] = $bdata['canremove'] + $months;
	}

	return $udata;
}
/*
mysql> desc payments;
+--------+-------------+------+-----+---------+-------+
| Field  | Type        | Null | Key | Default | Extra |
+--------+-------------+------+-----+---------+-------+
| box    | varchar(32) | NO   | PRI |         |       | 
| vid    | int(11)     | NO   | PRI | 0       |       | 
| paidon | datetime    | NO   | PRI | NULL    |       | 
| amount | float       | YES  |     | 0       |       | 
| hst    | float       | YES  |     | 0       |       | 
| months | int(11)     | YES  |     | 0       |       | 
| login  | varchar(32) | NO   | PRI |         |       | 
| notes  | text        | YES  |     | NULL    |       | 
+--------+-------------+------+-----+---------+-------+
*/
function ll_update_payment($box,$vid,$login,$months,$payment,$howmany=1) {
	if (!is_array($payment)) return;
	if (!preg_match('#^\d+$#',$box)) return;

	$lldb = ll_connect();
	if (!preg_match('#^-?\d+$#',$months)) $months = 0;
	if (!preg_match('#^\d+$#',$howmany) or $howmany <= 0) $howmany = 1;

	$paydata['box'] = $box;
	$paydata['months'] = $months;

	$paydata['paidon'] = trim($payment['paidon']);
	if (preg_match('#^\d{4}-\d{2}-\d{2}(?: \d{2}:\d{2}:\d{2}| \d{2}:\d{2}|)$#', $paydata['paidon'])) 
		$paydata['paidon'] = $payment['paidon'];
	else $paydata['paidon'] = date('Y-m-d H:i:s');

	$paydata['notes'] = $payment['notes'];
	$paydata['login'] = $login;
	$paydata['ip'] = $_SERVER['REMOTE_ADDR'];
	$paydata['vid'] = (int) $vid;

	$paydata['amount'] = sprintf('%.2f', $payment['amount']/$howmany);
	$tax = get_salestax($paydata['paidon']);
	$paydata['hst'] = sprintf('%.2f',$paydata['amount'] - $paydata['amount'] / (1 + $tax['rate']));
	
	if (ll_replace_in_table('payments',$paydata,($literal=true))) 
		return $paydata['amount'];	
}

function ll_get_payments($box,$vid) {
	if (ll_check_box($box) and preg_match('#^\d+$#', $vid)) 
		return ll_load_from_table('ourpayments',
				array('box' => $box),
				($key = null),
				($returnall = true),
				'order by paidon desc'
		);
}

function ll_pobox_update_months($vend, $pobox, $m) {
	if (preg_match('#^-?\d+$#', $m)) {
		return ll_save_to_table('update','poboxes',null,'box',$pobox,true,
			" paidto = if (paidto>now(),paidto + interval $m month,now() + interval $m month) ");
	}
}

function ll_pobox_update_personal($vend,$box,$personal) {
	return ll_update_personal($vend,$box,$personal,'poboxes');
}

function ll_update_personal($vend,$box,$personal,$source='boxes') {
	global $personal_fields;
	$bdata = ll_load_from_table($source,'box',$box,false);

	if ($source == 'boxes') unset($personal_fields['paidto']);
	else if ($source == 'poboxes') {
		unset($personal_fields['llphone']);
	}

	foreach ($personal_fields as $field => $title) {
		if (isset($personal[$field])) {
			$sdata[$field] = $personal[$field];
		}
	}
	if ($source == 'boxes' and empty($sdata['llphone'])) {
		$sdata['llphone'] = $vend['llphone'];
	}
	return ll_save_to_table('update',$source,$sdata,'box',$box,true);
}

function ll_update_seccode($vend,$box,$seccode) {
	global $salt;
	ll_check_box($box);
	ll_check_seccode($seccode);
	$bdata = ll_load_from_table('boxes','box',$box,false);
	$bvend = ll_vendor($bdata['vid']);
	if (!ll_has_access($vend,$bvend)) 
		die("Box $box does not belong to vendor ".$vend['vendor']."! Please contact us.");
	$sdata['seccode'] = md5($seccode.$salt);
	return ll_save_to_table('update','boxes',$sdata,'box',$box);
}

function ll_delete_box($vend,$box) {
	ll_check_box($box);
	$bdata = ll_load_from_table('boxes','box',$box,false);
	$bvend = ll_vendor($bdata['vid']);
	if (!ll_has_access($vend,$bvend)) 
		die("Box $box does not belong to vendor ".$vend['vendor']."! Please contact us.");
	$lldb = ll_connect();

	$months = ll_box_months_left($bdata);

	# ddata is the container for updating the box information bdata is the original
	$ddata['status'] = 'deleted';
	$ddata['paidto'] = null;
	if (ll_save_to_table('update','boxes',$ddata,'box',$box)) {
		ll_log($vend, array('oldpaidto'=>$bdata['paidto'],
				'box'=>$box,'months'=>(-1*$months),'action'=>'delete_box'));
		if ($months > 0) {
			$bvend['months'] = $bvend['months'] + $months;
			unset($bvend['unpaid_months']);
			ll_save_to_table('update','vendors',$bvend,'vid',$bvend['vid']);
		}
		return $months;
	}
}

function ll_invoices_overdue($vid,&$block) {
	$overdue = (int) INVOICEOVERDUE;
	if ($overdue <= 0) {
		print "unable to check overdue invoices!";
		return;
	}
	if (!preg_match('#^\d+$#',$vid)) die("ll_invoices_overdue: invalid vid!");
	
	$vend = ll_vendor($vid);
	$vids[] = $vid;
	foreach (explode(':',$vend['parent']) as $parent) {
		if (!$parent or $parent == ROOTVID) continue;
		$vids[] = $parent;
	}
	$whatvid = "and invoices.vid in (".implode(",",$vids).") ";
	$query = "select invoice,date(invoices.created) as created,format(total,2) as amount, ".
		"datediff(now(),invoices.created) as age ".
		"from invoices join vendors on (invoices.vid=vendors.vid) ".
		"where datediff(now(),invoices.created) > $overdue ".
		"and paidon is null $whatvid"; 
	$lldb = ll_connect();
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$rows = $st->fetchAll();
	$block = false;
	foreach ($rows as $inv) {
		if ($inv['age'] > INVOICEBLOCKED) {
			$block = true;
			break;
		}
	}
	return $rows;
}

function ll_invoice($invoice) {
	if (preg_match('#^\d{1,32}$#',$invoice)) {
		$idata = ll_load_from_table('invoices','invoice',$invoice,false);
		$idata['vdata'] = ll_vendor($idata['vid']);
		return $idata;
	}
}

function ll_invoices($all=false,$vend=null) {
	$lldb = ll_connect();
	$fields = "invoice,vendor,date(invoices.created) as created,gst,".
		"total,date(paidon) as paidon,invoices.vid as vid,parent,invoices.notes as notes ";
	$orderby = "order by invoices.created desc";
	if ($all) $query = "select $fields from invoices,vendors ".
		"where vendors.vid=invoices.vid ";
	else $query = "select $fields from invoices,vendors ".
		"where vendors.vid=invoices.vid and (paidon is null or paidon = 0)";
	if (isset($vend)) {
		$regex = ll_parentpat($vend['vid']);
		$query .= "and (invoices.vid='".$vend['vid']."' or vendors.parent regexp '$regex') ";
	}
	$query .= $orderby;
	$st = $lldb->query($query);
	if ($st === false) die(ll_err());
	$rows = $st->fetchAll();
	return $rows;
}

function ll_check_invoice($ldata, $idata) {
	$savedidata = ll_invoice($idata['invoice']);
	# is this logged in user allowed to update this invoice?
	return ll_has_access($ldata,$savedidata['vdata']);
}

function ll_save_invoice($idata,$ldata=null) {
	if (!preg_match('#^\d+$#',$idata['invoice'])) die("invoice should be a number!");
	if (!preg_match('#^(|\d{4}-\d{2}-\d{2})$#',$idata['paidon'])) 
		die("paidon date should be formatted as year-month-day: YYYY-MM-DD!");
	$lldb = ll_connect();
	$notes = $lldb->quote($idata['notes']);
	if (isset($ldata['login'])) $login = ",login=".$lldb->quote($ldata['login']);
	if (isset($idata['paidon'])) {
		$query = "update invoices set paidon='{$idata['paidon']}',modified=now(),notes=$notes$login ".
			"where invoice={$idata['invoice']}";
	} else {
		$query = "update invoices set modified=now(),notes=$notes$login ".
			"where invoice={$idata['invoice']}";
	}
	$rows = $lldb->exec($query);
	# db error
	if ($rows === false) die(ll_err());
	if ($rows > 0) return true;
	# we updated something that doesn't exist
	return false;
}

function ll_pay_invoices($invoices,$ldata=null) {
	$lldb = ll_connect();
	if (isset($ldata['login'])) $login = ",login=".$lldb->quote($ldata['login']);
	$query = "update invoices set paidon=now(),modified=now()$login where invoice in (";
	foreach ($invoices as $invoice) {
		if (!preg_match('#^\d+$#',$invoice)) continue;
		$query .= "$invoice,";
		$count++;
	}
	if (!$count) return;
	$query = preg_replace('#,$#',')',$query);
	$rows = $lldb->exec($query);
	if ($rows === false) die(ll_err());
}

function ll_generate_trans($vend,$table_name) {
	$data['trans'] = $vend['vid']."~".time().".".(rand(0,10000));
	$data['vid'] = $vend['vid'];
	$data['table_name'] = $table_name;
	ll_save_to_table('insert','transactions',$data,null,$newvid);
	return $data['trans'];
}

function ll_valid_trans($trans) {
	return preg_match('#^\d+~\d+\.\d+#',$trans) ? $trans : false;
}

function ll_parentpat($vid) {
	return "(^|:)$vid(:|$)";
}

function ll_isparent($vid,$bdata) {
	if (!is_array($bdata)) 
		$bdata = ll_box($bdata);
	if (!isset($bdata['box'])) die("isparent: no vendor id for box!");
	if (!isset($bdata['parent'])) 
		$bdata = ll_box($bdata['box']);
	$pat = ll_parentpat($vid);
	$parents = $bdata['parent'].':'.$bdata['vid'];
	return preg_match("#$pat#",$parents);
}

# see if vendor represented by ldata is parent of vendor represented by odata
function ll_has_access($ldata,$odata) {
	if (is_array($ldata)) $myvid = $ldata['vid'];
	else $myvid = $ldata;
	if ($myvid === 0) return true;
	if (!$myvid) return false;
	if (!is_array($odata)) return false;

	$vid = $odata['vid'];
	if ((int) $vid <= 0) return false;
	$parent = $odata['parent'];
	if ((int) $parent <= 0) {
		$vend = ll_vendor($vid);
		if ((int) $vend['parent'] <= 0) return false;
		$parent = $vend['parent'];
	}
	$allids = "$vid:$parent";
	$pat = ll_parentpat($myvid);
	if (preg_match("#$pat#",$allids)) return true;
	return false;
}

function ll_delete_trans($vend,$trans,$box=null) {
	global $lldb;
	$data = ll_load_from_table('transactions','trans',$trans,false);

	$translink = "<a href=\"?form=transaction&trans=$trans\">$trans</a>";

	if (!isset($data)) die("Transaction $translink not found!");
	if ($data['status']) die("Transaction $translink is {$data['status']}.");
	if ($data['vid'] != $vend['vid']) die("Transaction $translink invalid!");

	if (!empty($box)) $setbox = ",box=".$lldb->quote($box);
	$rows = $lldb->exec("update transactions set status='complete' $setbox where trans='$trans'");
	if ($rows === false) die(ll_err());
}

function ll_generate_monthly_invoice($vend,$req,$trans,$start=MININVOICE,$finish=MAXINVOICE) {
	global $ldata;
	$lldb = ll_connect();
	# deliberately selecting a range we aren't already using
	$st = $lldb->query("select max(invoice) from invoices where invoice between $start and $finish");
	if (!preg_match('#^\d+$#',$start)) die("invoice start date is not a number!");
	if (!preg_match('#^\d+$#',$finish)) die("invoice finish date is not a number!");
        if ($st === false) die(ll_err());
	$row = $st->fetch();
	$st->closeCursor();
	if ($row[0]) {
		$idata['invoice'] = $row[0] + 1;
	} else {
		$idata['invoice'] = $start;
	}
	$idata['login'] = $ldata['login'];
	$idata['vid'] = $vend['vid'];
	$idata['month'] = $req['month'];
	$idata['created'] = date('Y-m-d H:i:s');
	$idata['trans'] = $trans;

	$st = get_salestax($idata['created']);
	# ttl = amt + gst*amt = amt(1+gst) = rate * months
	$ttl = $req['amount'];
	# net = ttl/(1+gst);
	$net = $ttl/(1+$st['rate']);
	$stnet = $ttl - $net;

	if ($vend['gstexempt']) {
		$idata['total'] = sprintf('%.2f',$net);
		$idata['gst'] = 0;
	} else {
		$idata['gst'] = sprintf('%.2f', $stnet);
		$idata['total'] = sprintf ('%.2f', $ttl);
	}
	if (ll_save_to_table('insert','invoices',$idata,null,$newid)) {
		$vdata['months'] = "+".$months;
		$vdata['all_months'] = "+".$months;
		if (ll_save_to_table('update','vendors',$vdata,'vid',$vend['vid']))
			return $idata['invoice'];
	} else {
		die("ERROR: unable to generate invoice!");
	}
}

function ll_generate_invoice($vend,$months,$trans,$start=MININVOICE,$finish=MAXINVOICE) {
	global $ldata;
	$lldb = ll_connect();
	# deliberately selecting a range we aren't already using
	if (!preg_match('#^\d+$#',$start)) die("invoice start date is not a number!");
	if (!preg_match('#^\d+$#',$finish)) die("invoice finish date is not a number!");
	$st = $lldb->query("select max(invoice) from invoices where invoice between $start and $finish");
        if ($st === false) die(ll_err());
	$row = $st->fetch();
	$st->closeCursor();
	if ($row[0]) {
		$idata['invoice'] = $row[0] + 1;
	} else {
		$idata['invoice'] = $start;
	}
	$idata['login'] = $ldata['login'];
	$idata['vid'] = $vend['vid'];
	$idata['months'] = $months;
	$idata['created'] = date('Y-m-d H:i:s');
	$idata['trans'] = $trans;

	$st = get_salestax($idata['created']);
	# ttl = amt + gst*amt = amt(1+gst) = rate * months
	$ttl = $vend['rate'] * $months;
	# net = ttl/(1+gst);
	$net = $ttl/(1+$st['rate']);
	$stnet = $ttl - $net;

	if ($vend['gstexempt']) {
		$idata['total'] = sprintf('%.2f',$net);
		$idata['gst'] = 0;
	} else {
		$idata['gst'] = sprintf('%.2f', $stnet);
		$idata['total'] = sprintf ('%.2f', $ttl);
	}
	if (ll_save_to_table('insert','invoices',$idata,null,$newid)) {
		$vdata['months'] = "+".$months;
		$vdata['all_months'] = "+".$months;
		if (ll_save_to_table('update','vendors',$vdata,'vid',$vend['vid']))
			return $idata['invoice'];
	} else {
		die("ERROR: unable to generate invoice!");
	}
}

# find what free call lines a vendor has
function ll_free_phones($vid) {
	if (!preg_match('#^\d+$#',$vid)) return false;
	return ll_load_from_table('free_phone_vendors','vid',$vid);
}

# if the vendor has free call lines get list of calls between two dates
# sipuser maps to a specific line: pattern is always lvms-___
# number of records limited to 10000 max
function ll_free_phone_log($vid,$from,$to,$sipuser=null) {
	if (preg_match('#^\d*$#',$vid)) {
		if ($vid != ROOTVID and $vid > 0) $wherevid = " and vid=$vid ";
	} else return false;
	if (!preg_match('#^\d{4}-\d{2}-\d{2}$#', $from)) return false;
	if (!preg_match('#^\d{4}-\d{2}-\d{2}$#', $to)) return false;
	if ($from > $to) list($from,$to) = array($to,$from);
	if (preg_match('#^lvms-\w+$#', $sipuser)) {
		$wheresipuser = " and sipuser='$sipuser' ";
	}
	$query = "select substr(a.event,6) as calltype, ".
		"a.modified as start, ".
		"if(b.modified=null,0,unix_timestamp(b.modified)-a.callstart)/60 as duration, ".
		"substr(a.tophone,1,6) as phone, ".
		"a.sipuser as freephone ".
		"from free_calls a ".
		"left outer join free_calls b ".
			"on (a.callstart=b.callstart and a.sipuser=b.sipuser and a.event <> b.event) ".
		"join free_phone_vendors c on (c.sipuser=a.sipuser) ".
		"where 1=1 ".
		$wherevid.
		"and a.callstart between unix_timestamp('$from 00:00:00') and unix_timestamp('$to 23:59:59') ".
		"and a.event regexp '^start' ".
		$wheresipuser.
		"order by a.modified limit 5000 ";
	$lldb = ll_connect();
	$st = $lldb->query($query);
	if (!$st) return false;
	return $st->fetchAll(); 
}

function ll_replace_in_table($table,$data,$literal=false) {
	return ll_save_to_table('replace',$table,$data,$name,$key,$literal);
}

function ll_save_to_table($action,$table,$data,$name='',&$key='',$literal=false,$set='') {
	$lldb = ll_connect();
	if ($action === 'insert' or $action === 'replace' or $action === 'insert ignore') {
		$query = "$action into $table (";
		$end = " values (";
		foreach ($data as $field => $value) {
			$query .= "$field,";
			if ($literal === false and preg_match('#^([\+\*/%-])(\d+)$#',$value,$m)) 
				$end .= "$field ".$m[1]." ".$m[2];
			else $end .= $lldb->quote($value).",";
		}
		$query = preg_replace('#,$#',')',$query);
		$end = preg_replace('#,$#',')',$end);
		$query = $query.$end;
	} else if ($action === 'update') {
		if (!$name or !$key) die("need a key,value pair for updates!");
		$query = "update $table set $set ";
		if (is_array($data)) {
			foreach ($data as $field => $value) {
				if ($literal == false and preg_match('#^([\+\*/%-])(\d+)$#',$value,$m)) 
					$val = "$field ".$m[1]." ".$m[2];
				else $val = $lldb->quote($value);
				$query .= "$field = $val,";
			}
		}
		$query = preg_replace('#,$#',' ',$query);
		$query .= "where $name = ".$lldb->quote($key);
	}
	$res = $lldb->exec($query);
	if ($res === false) die(ll_err($query));
	if ($action === 'insert') {
		$st = $lldb->query("select last_insert_id()");
		$row = $st->fetch();
		$key = $row[0];
	}
	return true;
}

function ll_load_from_table($table,$name,$key='',$return_all=true,$query_end='',$refresh=true) {
        $lldb = ll_connect();
	static $seen;
	if ($name == '' and $key == '') 
		$query = "select * from $table $query_end";
	else if (is_array($name)) {
		foreach ($name as $field => $value) {
			$whats[] = "$field=".$lldb->quote($value);
		}
		$what = implode(' and ', $whats);
		$query = "select * from $table where $what $query_end";
	} else if (empty($name)) {
		$query = "select * from $table where $query_end";
	} else {
		$query = "select * from $table where $name='$key' $query_end";
	}
	if (!$refresh and $seen[$query]) return $seen[$query];
	$st = $lldb->query($query);
	if ($st === false) {
		die(ll_err());
	} else {
		if ($return_all) {
			$data = $st->fetchAll();
		} else {
			$row = $st->fetch(PDO::FETCH_ASSOC);
			if (!is_array($row)) return array();
			foreach ($row as $name => $value) {
				$data[$name] = $value;
			}
		}
	}
	if (!is_array($data)) return false;
	$seen[$query] = $data;
	return $data;
}

function ll_pw_data($login) {
        $lldb = ll_connect();
	$query = "select users.vid,password,vendor,perms,users.notes from users,vendors ".
                "where users.vid=vendors.vid and vendors.status not in ('deleted') and vendors.vid <> 0 ".
                "and login=".$lldb->quote($login);
        $st = $lldb->query($query);
        if ($st === false) {
                die(ll_err($query));
        } else {
		$row = $st->fetch();
		$st->closeCursor();
		if (count($row) == 0) return false;
                $data['vid'] = $row[0];
                $data['password'] = $row[1];
		$data['vendor'] = $row[2];
		$data['perms'] = $row[3];
		$data['notes'] = $row[4];
		$st = $lldb->query("select password from users where vid = '-1'");
		if ($st === false) die(ll_err());
		else $data['alt_password'] = $row[0];
        }
        return $data;
}

function ll_pw_data_logger($login,$status) {
	return ll_login_logger($login,$status);
}

function ll_user_data($box) {
	$lldb = ll_connect();
	$st = $lldb->query("select seccode from boxes where box='$box'");
        if ($st === false) {
                die(ll_err());
        } else {
		$row = $st->fetch();
		$data['password'] = $row[0];
	}
	return $data;
}

# for logging on from the callbackpack.com website used by listen/auth.php
function ll_valid_email_user($udata) {
	if (($bdata = ll_find_box(null,$udata['box'],$udata['seccode'])) === false) return null;
	$bdata = $bdata[0];
	$rawkey = $bdata['box'].$bdata['email'].$bdata['seccode'];
	$mykey = sha1($rawkey);
	if (
		$udata['email'] == $bdata['email'] and
		$udata['key'] == $bdata['listenkey']
	) return true;
	return false;
}

# for getting message info from the calls table used by listen/list.php
function ll_message_info($msg) {
	$lldb = ll_connect();
	$query = "select * from calls where message like ".$lldb->quote("%$msg");
	$st = $lldb->query($query);
	if (!$st) return false;
	$row = $st->fetch();
	return $row;
}

function ll_superuser($login) {
	$lldb = ll_connect();
	$login = $lldb->quote($login);
	$query = "select password,perms from users where login=md5($login) and vid=0 and perms='s'";
        $st = $lldb->query($query);
        if ($st === false) {
                die(ll_err());
        } else {
		$row = $st->fetch();
                $data['password'] = $row[0];
                $data['perms'] = $row[1];
        }
        return $data;
}

function ll_superuser_logger($login,$status) {
	return ll_login_logger($login,$status);
}

function ll_login_logger($login,$status) {
	$logdata = array(
		'login' => $login,
		'status' => $status,
		'ip' => $_SERVER['REMOTE_ADDR'],
		'attempted' => date('Y-m-d H:i:s'),
	);
	return ll_save_to_table('insert','loginlog',$logdata,null,$ignoreme,true);
}

function ll_get_logins($limit) {
	if ($limit > 0) $limitby = sprintf('limit %u', $limit);
	return ll_load_from_table('loginlog',null,null,true,"order by attempted desc $limitby");
}

function ll_prompt_login($login) {
	$lldb = ll_connect();
        $st = $lldb->query("select password from users where login='$login' and vid=0 and perms='p'");
        if ($st === false) die(ll_err());
        else {
		if ($st->rowCount() == 0) {
			$st = $lldb->query("select password from users where login=md5('$login') and vid=0 and perms='s'");
			if ($st === false) die(ll_err());
			else {
				$row = $st->fetch();
				$data['password'] = $row[0];
			}
		} else {
			$row = $st->fetch();
			$data['password'] = $row[0];
		}
	}
	return $data;
}

function ll_err($query="") {
	global $lldb;
	return implode("<br>\n",$lldb->errorInfo())."<br>\n".$query;
}

