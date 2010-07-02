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
	if (isset($_SESSION['login'])) return $_SESSION['login'];
	$login = $_REQUEST['login'];
	if (!preg_match("#^\S{1,32}$#",$login)) return;
	$password = $_POST[password];
	if ($password == '' or strlen($password) > 50) return;
	$password = md5($password);
	$ldata = $callback($login);
	if ($ldata['password'] === $password or $ldata['alt_password'] === $password) {
		$ldata = save_login($login,$app,$ldata);
		return $ldata;
	}
	return;
}

function delete_login() {
	unset($_SESSION['login']);
}

function save_login($this_login,$this_app,$ldata) {
	$_SESSION['login'] = $ldata;
	$_SESSION['login']['app'] = $this_app;
	$_SESSION['login']['login'] = $this_login;
	$_SESSION['login']['time'] = $time = time();
	return $_SESSION['login'];
}

