# 初始化相关
## 用户添加

```shell
# 进入mysql
CREATE USER 'game'@'%' IDENTIFIED BY '123456';
GRANT ALL PRIVILEGES ON *. * TO 'game'@'%';
flush privileges;
```

如果需要修改密码，使用下面的语句（新版本不再使用PASSWORD函数，可以直接使用字符串）
```shell
SET PASSWORD FOR 'game'@'%' = PASSWORD('123456');
ALTER USER game@'%' IDENTIFIED BY '123456';
```

## 创建数据库和表
可能修改脚本中名字
./createAllTables.sh

## 创建单表
mysql -ugame -p123456 gameserver_db < tables/A.sql

# 修改后备份
## 导出所有表结构
mysqldump -d -h 127.0.0.1 -ugame -p123456 gameserver_db > createTables.sql

## 指定单独表
mysqldump -d -h 127.0.0.1 -ugame -p123456 gameserver_db --table A > tables/A.sql
