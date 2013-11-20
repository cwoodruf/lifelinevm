<?php
require_once('twitterapi.php');
// not needed for lifeline
// require_once('twitter_db.php');

class Twitter 
{
	private static $ta;

	public static $actions = array(
		'tweet' => 'twitter_update',
		'deltweet' => 'twitter_destroy',
		'timeline' => 'twitter_timeline',
	);

	static function execute($action) 
	{
		if (self::$actions[$action] !== false) {
			$callback = self::$actions[$action];
			self::$ta = new TwitterAPI;
			return self::$callback($_REQUEST);
		}
		return false;
	}

	static function screen_name() 
	{
		return TWITTER_SCREEN_NAME;
	}

	private static function twitter_update($r) 
	{
		return self::db(self::$ta->tweet($r['tweet']),
				array('callback'=>'twitter_set_tweet',
					'pid'=>$r['pid'],'lang'=>$r['lang']));
	}

	private static function twitter_destroy($r)
	{
		return self::db(self::$ta->destroy($r['tweetid']),
				array('callback'=>'twitter_set_deleted'));
	}

	private static function twitter_timeline($r)
	{
		return self::db(self::$ta->timeline());
	}
	
	public static function db($json, $params=null) 
	{
		if (is_array($params)) $callback = $params['callback'];
		try {
			$o = json_decode($json);
			if (!isset($o->errors) and $callback) {
				if (function_exists($callback)) {
					$o->pid = $params['pid'];
					$o->lang = $params['lang'];
					$err = $callback($o);
					if ($err) throw new Exception("$callback $err");
				} else {
					throw new Exception("$callback callback not found!");
				}
			}
			return $json;
		} catch (Exception $e) {
			return json_encode(
				array(
					'errors'=>array(
						array( 'message'=>$e->getMessage() ),
					)
				)
			);
		}
	}
}

