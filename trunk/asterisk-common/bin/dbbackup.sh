#!/bin/sh
cd /usr/local/asterisk
/usr/bin/mysqldump --opt -ull -p'85$82$str' lifeline > lifeline.mysql
/usr/bin/find dbbackups -name 'lifeline*.mysql.gz' -mtime +30 -exec /bin/rm {} \;
/bin/gzip -c lifeline.mysql > dbbackups/lifeline-`/bin/date +%Y%m%d`.mysql.gz

