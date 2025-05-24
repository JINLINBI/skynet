if [ $# != 2 ] ; then
echo "bad param!"
echo " e.g.: ./createAllTables.sh 127.0.0.1 3306"
exit 1;
fi
echo $0
echo $1
mysql -h$1 -P$2 -uroot -p"123456" < createUser.sql;
echo "finished create user game"
mysql -h$1 -P$2 -ugame -p"123456" < createDB.sql;
echo "finished create database gameserver_db"
for SQL in tables/*.sql; do mysql -h$1 -P$2 -ugame -p"123456" gameserver_db < $SQL; done
