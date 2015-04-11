<?php
# all of this stuff is now in $lib/mysql.php with "ll_" prepended to the functions

# update other tables related to a voicemail box
function sync_client() {
	global $vdata;
	$personal = get_client();
	$cid = $personal['cid'];
	# the cid is necessary here to glue together the other records
	if ($cid > 0) {
		$pers = $personal;
		# do not overwrite existing notes in the poboxes record
		unset($pers['notes']);
		if ($_REQUEST['updatepoboxes']) {
			foreach ($_REQUEST['poboxes'] as $pobox) {
				if (ll_pobox_ok($pobox) != null) {
					ll_pobox_update_personal($vdata, $pobox, $pers);
					ll_pobox_update_months($vdata, $pobox, $_REQUEST['months']);
				}
			}
		} else if (is_array($pers) and 
				($pobox = ll_pobox_ok($_REQUEST['pobox'])) != null) {
			ll_pobox_update_personal($vdata, $pobox, $pers);
			ll_pobox_update_months($vdata, $pobox, $_REQUEST['months']);
		}
	}
	$_REQUEST['personal'] = $personal;
	return $cid;
}

# get or create a client record using the cid or name
function get_client() {
	global $vdata;
	# use the name, notes fields to update the clients and poboxes tables
	# use the vm box as the basis for this
	if (!isset($_REQUEST['personal'])) {
		$cid = $_REQUEST['cid'];
		$personal = ll_box($_REQUEST['box']);
		unset($personal['paidto']);
	} else {
		$cid = $_REQUEST['personal']['cid'];
		$personal = $_REQUEST['personal'];
	}
	# find or create a clients record
	if (preg_match('#^\d+$#', $cid)) {
		$cdata = ll_client($cid);
		if (!$cdata) $cid = null;
		else {
			$personal['cid'] = $cid;
			ll_client_update($vdata, $personal);
		}
	} else $cid = null;
	# with no explicit cid we should check the name first
	# then make a new record if the name can't be found
	if ($cid == null) {
		if (isset($personal['name'])) {
			$cdata = ll_client_from_name($personal['name']);
			if ($cdata === false or !isset($cdata['cid'])) {
				$cid = ll_client_insert($vdata, $personal);
			} else {
				$cid = $cdata['cid'];
				$personal['cid'] = $cid;
				ll_client_update($vdata, $personal);
			}
		}
	}
	# if we get to this point we should have a cid
	$personal['cid'] = $cid;
	$personal['vid'] = $vdata['vid'];
	return $personal;
}

