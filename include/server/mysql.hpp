#ifndef MYSQL_H
#define MYSQL_H

#include <mysql/mysql.h>
#include <string>
using namespace std;

class Mysql
{
public:
    //初始化数据库连接
    Mysql();
    //释放数据库连接资源
    ~Mysql();
    //连接数据库
    bool connect();
    //查询
    MYSQL_RES *query(string sql);
    //更新
    bool update(string sql);
    //获取连接
    MYSQL* getConnection();
private:
    MYSQL* connection;
};

#endif