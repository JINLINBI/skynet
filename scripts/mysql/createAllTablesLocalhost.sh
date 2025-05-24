#!/bin/bash

mysql -hlocalhost --protocol=tcp < createUser.sql;
echo "finished create user game"
mysql -ugame -p123456 --protocol=tcp < createDB.sql;
echo "finished create database game"
for SQL in tables/*.sql; do mysql  -ugame -p123456 --protocol=tcp  gameserver_db < $SQL; done
