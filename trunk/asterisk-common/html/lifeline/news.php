<?php
# another program asterisk/bin/gettweets.pl makes the tweets.serialized file
function gettweets() {
	$news = unserialize(file_get_contents('tweets.serialized'));
	krsort($news);
	if (is_array($news)) {
		$newsdiv = <<<HTML
<div class="news">
<h4>News from our <a href="http://twitter.com/lifelinevm">Twitter</a> feed:</h4>
<ul>
HTML;
		foreach ($news as $datetime => $item) {
			$date = substr($datetime,1,10);
			$newsdiv .= "<li class=\"news\"><b>$date</b> - $item</li>\n";
		}
	$newsdiv .= <<<HTML
</ul>
</div>
HTML;
	}
	return $newsdiv;
}

