#!/bin/sh
# IMPORTANT: this script should only be run on the main asterisk server 
#
# in the event of an outage the other servers will have messages on them
# if this becomes routine we'd need to use the db to save message data
# TODO find a way to recognize when an caller has deleted messages on a backup
# the modification date doesn't seem to get changed when they do a delete??

for msgdir in /usr/local/asterisk/lifeline-msgs/ /usr/local/asterisk/coolaid-msgs/
do
	us=asterisk@aibackup.dyndns.org
	findcmd="/usr/bin/find $msgdir -regex '.*\(\.[0-9]+\|greeting\)\.gsm' \
			-newer ~/.lastupdated -print -exec /usr/bin/scp --preserve {} $us:{} \;"
	echo start `/bin/date`
	echo $findcmd 
	lastupdate="/bin/touch ~/.lastupdated"
	localbackup=10.2.170.10
	remotebackup=66.152.64.75

	echo sync-msgs: FROM $localbackup TO main $msgdir using ssh
	/usr/bin/ssh asterisk@$localbackup "$findcmd"
	/usr/bin/ssh asterisk@$localbackup "$lastupdate"

	echo sync-msgs: FROM $remotebackup TO main $msgdir using ssh
	/usr/bin/ssh asterisk@$remotebackup "$findcmd"
	/usr/bin/ssh asterisk@$remotebackup "$lastupdate"

	echo sync-msgs: FROM main $msgdir TO $localbackup using rsync
	/usr/bin/rsync -rcvtl --delete --times $msgdir asterisk@$localbackup:$msgdir
	#/usr/bin/rsync -rcvtl --times $msgdir asterisk@$localbackup:$msgdir
	echo sync-msgs: FROM main $msgdir TO $remotebackup using rsync
	#/usr/bin/rsync -rcvtl --delete --times $msgdir asterisk@$remotebackup:$msgdir
	/usr/bin/rsync -rcvtl --times $msgdir asterisk@$remotebackup:$msgdir
done

echo sync-msgs: CLEANUP `/bin/date`
/usr/local/asterisk/bin/cleanup.pl -v
# don't need to do this with sync as normally all call data goes to the master db
# also there is a problem where data is getting repeated because the florida server is 3 hours ahead and
# mysql doesn't seem to keep the same time when doing slave updates 
# echo sync-msgs: syncronize the calls table with any new callerid data etc
# /usr/local/asterisk/bin/sync_calls.pl -v

echo send email alerts for new messages
/usr/local/asterisk/bin/llremindinit.pl

/bin/touch ~/.lastupdated
