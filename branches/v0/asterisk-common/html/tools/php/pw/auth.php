<?php
session_start();
require_once("$lib/pw/forms.php");
require_once("$lib/mysql.php");
$cachedir = "$lib/pw/cache";
$cache = '';

function login_response($redirecturl,$app,$callback) {
	global $cache;
	$login = authenticate($app,$callback);
	if (isset($login)) return $login;
	print_login($redirecturl,$app,$callback);
	exit;
}

function authenticate($app,$callback) {
	if (is_array($_SESSION['login'])) return $_SESSION['login'];
	$login = $_REQUEST['login'];
	if (!preg_match("#^\S{1,64}$#",$login)) return;
	$password = $_REQUEST[password];
	if ($password == '' or strlen($password) > 50) return;
	$password = md5($password);
	if (function_exists($callback)) {
		$ldata = $callback($login);
	} else {
		return;
	}
	$loginok = ($ldata['password'] === $password or $ldata['alt_password'] === $password) ? true : false;
	if (function_exists($logger = $callback.'_logger')) {
		$logger($login, ($loginok ? 'ok' : 'failed'));
	}
	if ($loginok) {
		$ldata = save_login($login,$app,$ldata);
		return $ldata;
	}
	return;
}

function delete_login() {
	unset($_SESSION['login']);
	unset($_COOKIE['from']);
	setcookie('from',null,mktime(0,0,0,1,1,1970));
}

function save_login($this_login,$this_app,$ldata) {
	$_SESSION['login'] = $ldata;
	$_SESSION['login']['initial_vid'] = $ldata['vid'];
	$_SESSION['login']['app'] = $this_app;
	$_SESSION['login']['login'] = $this_login;
	$_SESSION['login']['time'] = time();
	return $_SESSION['login'];
}

