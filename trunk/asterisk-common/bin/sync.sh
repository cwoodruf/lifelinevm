#!/bin/bash
if [ $HOSTNAME != aifl89.ath.cx ] ; then
	echo -$HOSTNAME: running rsync on aifl.ath.cx
	/usr/bin/ssh asterisk@aifl.ath.cx '/usr/local/asterisk/bin/sync.sh'
fi
echo start rsync `/bin/date`
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/html/ /usr/local/asterisk/html
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/paycode/ /usr/local/asterisk/paycode
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/agi-bin/ /usr/local/asterisk/agi-bin
/usr/bin/rsync -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/bin/ /usr/local/asterisk/bin
/usr/bin/rsync --delete -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/Lifeline/ /usr/local/asterisk/Lifeline
/usr/bin/rsync -rcvt asterisk@lifelinevm.net:/usr/local/asterisk/lifeline-msgs/ /usr/local/asterisk/lifeline-msgs
echo start scp `/bin/date`
# /usr/bin/scp asterisk@lifelinevm.net:/usr/local/asterisk/lifeline*.zip /usr/local/asterisk/
/usr/bin/scp asterisk@lifelinevm.net:/usr/local/asterisk/lifeline.mysql /usr/local/asterisk/
echo updating database `/bin/date`
/bin/cat /usr/local/asterisk/lifeline.mysql | /usr/bin/mysql -ull -p'85$82$str' -Dlifeline
/usr/local/asterisk/bin/cleanup.pl -v

