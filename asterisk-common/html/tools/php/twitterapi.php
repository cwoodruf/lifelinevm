<?php
// see https://github.com/J7mbo/twitter-api-php
require_once('TwitterAPIExchange.php');

# provide different ways to store the twitter oauth info
# can also be set with the constructor
$twitter_constants = array(
	'ACCESS_TOKEN', 'ACCESS_TOKEN_SECRET', 'CONSUMER_KEY', 'CONSUMER_SECRET',
	'SCREEN_NAME',
);
foreach ($twitter_constants as $tconst) {
	$tconst = 'TWITTER_'.$tconst;
	if (!defined($tconst)) {
		if (isset($_SERVER[$tconst])) define($tconst,$_SERVER[$tconst]);
		else define($tconst, "");
	}
}

class TwitterAPI extends TwitterAPIExchange
{
	const TWEET_SIZE = 140;

	public $strict = false; # fail on unknown fields

	# has to be set up on twitter for the specific app
	# https://dev.twitter.com/apps/
	private $oauth_settings = array(
	    'oauth_access_token' => TWITTER_ACCESS_TOKEN,
	    'oauth_access_token_secret' => TWITTER_ACCESS_TOKEN_SECRET,
	    'consumer_key' => TWITTER_CONSUMER_KEY,
	    'consumer_secret' => TWITTER_CONSUMER_SECRET 
	);

	private $baseurl = 'https://api.twitter.com/1.1/';

	private $apis = array(
		'ids' => array(
			'url' => 'followers/ids.json',
			'method' => 'GET',
		),
		'tweet' => array(
			'url' => 'statuses/update.json',
			'method' => 'POST',
			'params' => array(
				'status' => array( 'required' => true, ),
				'in_reply_to_status_id' => array( 'required' => false, ),
				'trim_user' => array('required' => false, 'default' => 'true' ),
			),
		),
		'destroy' => array(
			'url' => 'statuses/destroy/:id.json',
			'method' => 'POST',
			'params' => array(
				'id' => array( 'required' => true, ),
				'trim_user' => array('required' => false, 'default' => 'true' ),
			),
		),
		'timeline' => array(
			'url' => 'statuses/user_timeline.json',
			'method' => 'GET',
			'params' => array(
				'screen_name' => 
					array( 'required' => false, 'default' => TWITTER_SCREEN_NAME),
				'trim_user' => 
					array('required' => false, 'default' => 'true' ),
				'user_id' => 
					array('required' => false),
			),
				
		),
	);

	function __construct($oauth=null) 
	{
		if (is_array($oauth)) {
			$this->oauth_settings = $oauth;
		}
		parent::__construct($this->oauth_settings);
	}
	
	# front end methods
	# other users 
	function ids() 
	{
		return $this->request('ids');
	}

	# update status
	function tweet($status) 
	{
		if (strlen($status) > self::TWEET_SIZE) {
			$this->error = "Tweet too long!";
			return false;
		}
		if (!$status) {
			$this->error = "No tweet found!";
			return false;
		}
		return $this->request('tweet', array('status'=>$status));
	}

	# delete a tweet
	function destroy($id) 
	{
		if (!preg_match('#^\d+$#', $id)) {
			$this->error = "Need a numeric tweet id!";
			return false;
		}
		return $this->request('destroy', array('id'=>$id));
	}

	# list tweets for some user - user can be defined via SetEnv in .htaccess
	function timeline($screen_name = null) 
	{
		if (isset($screen_name)) { 
			$data['screen_name'] = $screen_name;
		} 
		return $this->request('timeline', $data);
	}

	# shared methods
	function setapi($req) 
	{
		if (!is_array($this->apis[$req])) return;
		$this->api = $this->apis[$req];
		$this->method = $this->api['method'];
	}

	function oauth($req, $data) 
	{
		$url = $this->api['url'];
		# deal with url stuff that requires replacing
		if (preg_match_all('#:(\w+)#', $this->api['url'], $m)) {
			if (!is_array($data)) {
				throw new Exception(
					"TwitterAPI: no data array"
				);
			}
			foreach ($m[1] as $match) {
				if (!isset($data[$match])) {
					throw new Exception(
						"TwitterAPI: missing required url param $match"
					);
				}
				$url = preg_replace("#:$match\b#", $data[$match], $url);
			}
		}
		$this->url = $this->baseurl.$url;

		$this->buildOauth($this->url, $this->method);
	}

	function request($req, $data=null) 
	{
		try {
			$this->setapi($req);
			switch ($this->method) {
			case 'GET':
				$this->getfields($data);
			break;
			case 'POST':
				$this->postfields($data);
			break;
			default: 
				throw new Exception(
					"TwitterAPI: unknown method {$this->method}"
				);
			}
			$this->oauth($req, $data);
			return $this->performRequest();

		} catch (Exception $e) {
			die($e->getMessage());
		}
	}

	function fields($data)
	{
		if (!is_array($data)) $data = array();

		if (!is_array($this->api['params'])) 
			return ($this->strict ? array(): $data);

		foreach ($this->api['params'] as $field => $meta) {
			if (!isset($data[$field])) {
				if ($meta['default']) {
					$data[$field] = $meta['default'];
				} else if ($meta['required']) {
					throw new Exception(
						"TwitterAPI: missing required field $field!"
					);
				}
			}
		}

		foreach ($this->api['params'] as $field => $f) {
			if (isset($f['default'])) $f['value'] = $f['default'];
			$fields[$field] = $f;
		}
		if ($this->strict) return $fields;

		if (is_array($data)) {
			foreach ($data as $field => $value) {
				$fields[$field]['value'] = $value;
			}
		}
		return $fields;
	}

	function getfields($data) 
	{
		$fields = $this->fields($data);
		if (is_array($fields) and count($fields)) {
			foreach ($this->fields($data) as $field => $fdata) {
				if (!$fdata['value']) continue;
				$query[] = "$field={$fdata['value']}";
			}
			$this->query = "?".implode('&', $query);
			$this->setGetfield($this->query);
		}
	}

	function postfields($data)
	{
		$fields = $this->fields($data);
		if (is_array($fields) and count($fields)) {
			$this->query = array();
			foreach ($fields as $field => $fdata) {
				if (!$fdata['value']) continue;
				$this->query[$field] = $fdata['value'];
			}
			$this->setPostfields($this->query);
		}
	}
}

