#!/bin/sh
cd /usr/local/asterisk
/usr/bin/scp --preserve callbackpack10@callbackpack.com:~/paycode/*.csv /usr/local/asterisk/paycode
/usr/local/asterisk/bin/paycode-sync.pl

# now getting an rsync connection directly to here
# /usr/bin/rsync -rctvz --delete cal@aifl.ath.cx:~/Lifeline/ /usr/local/asterisk/Lifeline
/usr/local/asterisk/bin/users.pl
/usr/local/asterisk/bin/convertvox.pl

/usr/bin/mysqldump --opt -ull -p'85$82$str' lifeline > lifeline.mysql
/usr/bin/zip -r lifeline-backup.zip agi-bin html etc/asterisk/*.conf /home/cal/svn/asterisk-*/apps/app_recordkey*.c lifeline.mysql 
/usr/bin/scp lifeline-backup.zip cal@aifl.ath.cx:~
/usr/bin/scp lifeline-backup.zip callbackpack10@callbackpack.com:~
