#!/bin/sh
statefile=/home/asterisk/logrotate.state

/usr/local/asterisk/sbin/asterisk -rx 'logger rotate' >> /usr/local/asterisk/bin/checkregistry.out

for r in /usr/local/asterisk/bin/*.rotate 
do
	echo $r
	/usr/sbin/logrotate -s "$statefile" "$r"
done

