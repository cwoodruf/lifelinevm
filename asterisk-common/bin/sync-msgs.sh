#!/bin/sh
# in the event of an outage the other servers will have messages on them
# if this becomes routine we'd need to use the db to save message data
msgdir=/usr/local/asterisk/lifeline-msgs/
us=asterisk@lifelinevm.net
findcmd="/usr/bin/find $msgdir -regex '.*\(\.[0-9]+\|greeting\)\.gsm$' -newer ~/lastupdated -print -exec /usr/bin/scp --preserve {} $us:{} \;"
lastupdate="/bin/touch ~/lastupdated"

echo sync-msgs: FROM local backup TO main using ssh
/usr/bin/ssh asterisk@192.168.1.44 "$findcmd"
/usr/bin/ssh asterisk@192.168.1.44 "$lastupdate"

echo sync-msgs: FROM remote backup TO main using ssh
/usr/bin/ssh asterisk@aifl.ath.cx "$findcmd"
/usr/bin/ssh asterisk@aifl.ath.cx "$lastupdate"

echo sync-msgs: CLEANUP
/usr/local/asterisk/bin/cleanup.pl -v
echo sync-msgs: syncronize the calls table with any new callerid data etc
/usr/local/asterisk/bin/sync_calls.pl -v

echo sync-msgs: FROM main TO local backup with delete
/usr/bin/rsync -rcvt --delete --times $msgdir asterisk@192.168.1.44:$msgdir
echo sync-msgs: FROM main TO remote backup with delete
/usr/bin/rsync -rcvt --delete --times $msgdir asterisk@aifl.ath.cx:$msgdir
