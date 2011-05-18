#!/bin/sh
cd /usr/local/asterisk
/home/asterisk/mysql-bin/mysqldump --opt -ucal -p'd834hWRY' --host=127.0.0.1 --port=3308 lifeline > lifeline.mysql
/usr/bin/find dbbackups -name 'lifeline*.mysql.gz' -mtime +30 -exec /bin/rm {} \;
/bin/gzip -c lifeline.mysql > dbbackups/lifeline-`/bin/date +%Y%m%d`.mysql.gz

/home/asterisk/mysql-bin/mysqldump --opt -ucoolaid -p'XVm5aGU4n3VFx' --host=127.0.0.1 --port=3308 coolaid > coolaid.mysql
/usr/bin/find dbbackups -name 'coolaid*.mysql.gz' -mtime +30 -exec /bin/rm {} \;
/bin/gzip -c coolaid.mysql > dbbackups/coolaid-`/bin/date +%Y%m%d`.mysql.gz
