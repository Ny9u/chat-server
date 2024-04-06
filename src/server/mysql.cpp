#include "mysql.hpp"
#include  <muduo/base/Logging.h>
using namespace std;

static string server ="127.0.0.1";
static string user ="root";
static string password ="your password";
static string name="database name";

Mysql::Mysql()
{   
    connection=mysql_init(nullptr);
}

Mysql::~Mysql()
{
    if(connection!=nullptr)
    {
        mysql_close(connection);
    }
}

bool Mysql::connect()
{
    MYSQL*p=mysql_real_connect(connection,server.c_str(),user.c_str(),
    password.c_str(),name.c_str(),3306,nullptr,0);

    if(p!=nullptr)
    {
        mysql_query(connection,"set names utf8");
        LOG_INFO << "mysql work";
    }
    return p;
}

MYSQL_RES* Mysql::query(string sql)
{
    if(mysql_query(connection,sql.c_str()))
    {
        LOG_ERROR << __FILE__<<":"<< __LINE__<<":"<<sql<<" QUERY FAIL";
        return nullptr;
    }
    return mysql_use_result(connection);
}

bool Mysql:: update(string sql)
{
    if(mysql_query(connection,sql.c_str()))
    {
        LOG_ERROR << __FILE__<<":"<< __LINE__<<":"<<sql<<" UPDATE FAIL";
        return false;
    }
    return true;
}

MYSQL*Mysql::getConnection()
{
    return connection;
}