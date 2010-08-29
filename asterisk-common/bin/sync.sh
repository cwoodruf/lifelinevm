#!/bin/bash
/usr/bin/ssh asterisk "/usr/bin/svn update /home/asterisk/svn/lifelinevm"
if [ $HOSTNAME != aifl89.ath.cx ] ; then
	echo -$HOSTNAME: running rsync on aifl.ath.cx
	/usr/bin/ssh asterisk@aifl.ath.cx '/usr/local/asterisk/bin/sync.sh'
fi
echo start rsync `/bin/date`
/usr/bin/rsync --delete -rcvt --exclude='.*' asterisk@lifelinevm.net:/usr/local/asterisk/html/ /usr/local/asterisk/html
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/paycode/ /usr/local/asterisk/paycode
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/agi-bin/ /usr/local/asterisk/agi-bin
/usr/bin/rsync -rcvt --exclude='*.out' asterisk@lifelinevm.net:/usr/local/asterisk/bin/ /usr/local/asterisk/bin
# /usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/Lifeline/ /usr/local/asterisk/Lifeline
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/dbbackups/ /usr/local/asterisk/dbbackups
# see sync-msgs.sh
# /usr/bin/rsync -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/lifeline-msgs/ /usr/local/asterisk/lifeline-msgs
echo cleanup `/bin/date`
/usr/local/asterisk/bin/cleanup.pl -v
if [ -x /usr/bin/svn ] ; then
	echo updating the svn repository
	/usr/bin/svn update /home/asterisk/svn/lifelinevm
fi
