#!/bin/sh
# in the event of an outage the other servers will have messages on them
# if this becomes routine we'd need to use the db to save message data
msgdir=/usr/local/asterisk/lifeline-msgs/
us=asterisk@lifelinevm.net
findcmd="/usr/bin/find $msgdir -regex '.*\(\.[0-9]+\|greeting\)\.gsm$' -newer ~/lastupdated -print -exec /usr/bin/scp --preserve {} $us:{} \;"
lastupdate="/bin/touch ~/lastupdated"
localbackup=192.168.1.44
remotebackup=aifl.ath.cx

echo sync-msgs: FROM $localbackup TO main using ssh
/usr/bin/ssh asterisk@$localbackup "$findcmd"
/usr/bin/ssh asterisk@$localbackup "$lastupdate"

echo sync-msgs: FROM $remotebackup TO main using ssh
/usr/bin/ssh asterisk@$remotebackup "$findcmd"
/usr/bin/ssh asterisk@$remotebackup "$lastupdate"

echo sync-msgs: CLEANUP
/usr/local/asterisk/bin/cleanup.pl -v
# don't need to do this with sync as normally all call data goes to the master db
# also there is a problem where data is getting repeated because the florida server is 3 hours ahead and
# mysql doesn't seem to keep the same time when doing slave updates 
# echo sync-msgs: syncronize the calls table with any new callerid data etc
# /usr/local/asterisk/bin/sync_calls.pl -v

echo sync-msgs: FROM main TO $localbackup with delete
/usr/bin/rsync -rcvt --delete --times $msgdir asterisk@$localbackup:$msgdir
echo sync-msgs: FROM main TO $remotebackup with delete
/usr/bin/rsync -rcvt --delete --times $msgdir asterisk@$remotebackup:$msgdir

echo send email alerts for new messages
/usr/local/asterisk/bin/llremindinit.pl
