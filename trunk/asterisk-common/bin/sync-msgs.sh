#!/bin/sh
# in the event of an outage the other servers will have messages on them
# if this becomes routine we'd need to use the db to save message data
msgdir=/usr/local/asterisk/lifeline-msgs/

echo sync-msgs: FROM local backup TO main
/usr/bin/rsync -rcvt --times asterisk@192.168.1.44:$msgdir $msgdir
echo sync-msgs: FROM remote backup TO main
/usr/bin/rsync -rcvt --times asterisk@aifl.ath.cx:$msgdir $msgdir

echo sync-msgs: CLEANUP
/usr/local/asterisk/bin/cleanup.pl -v
/usr/bin/find $msgdir -name .deleted.gsm -exec /bin/rm {} \;

echo sync-msgs: FROM main TO local backup with delete
/usr/bin/rsync -rcvt --delete --times $msgdir asterisk@192.168.1.44:$msgdir
echo sync-msgs: FROM main TO remote backup with delete
/usr/bin/rsync -rcvt --delete --times $msgdir asterisk@aifl.ath.cx:$msgdir
