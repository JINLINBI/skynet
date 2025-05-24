# 一、部署 MySQL Exporter​​
## 1. ​​安装 Exporter​​
​​Docker 方式​​（推荐）：
```shell
docker run -d -p 9104:9104 --name mysql_exporter \
  -v .my.cnf:/etc/mysql-exporter.cnf \  # 本地配置文件路径
  -e DATA_SOURCE_NAME="exporter:123456@(192.168.1.15:3306)/" \
  prom/mysqld-exporter \
  --config.my-cnf=/etc/mysql-exporter.cnf  # 指定配置文件
```
替换 exporter（用户名）、password（密码）、mysql_host（MySQL 地址）。
​​手动安装​​（Linux 环境）：
```shell
wget https://github.com/prometheus/mysqld_exporter/releases/download/v0.17.2/mysqld_exporter-0.17.2.linux-amd64.tar.gz
tar -xvf mysqld_exporter-*.tar.gz
cp mysqld_exporter /usr/local/bin/
```
## 2. ​​配置 MySQL 用户权限​​
需在 MySQL 中创建专用监控用户并授权：
```shell
CREATE USER 'exporter'@'%' IDENTIFIED BY '123456';
GRANT PROCESS, REPLICATION CLIENT, SELECT ON *.* TO 'exporter'@'%';
FLUSH PRIVILEGES;
```
​​说明​​：PROCESS 权限用于获取线程状态，SELECT 用于查询性能指标。