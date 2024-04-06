#include "usermodel.hpp"
#include "mysql.hpp"
#include <iostream>
using namespace std;

bool UserModel::insert(User &user)
{
    char sql[1024];
    sprintf(sql,"insert into user (name,account,password,state) values ('%s',round(rand()*(999999999-100000000)+100000000),'%s','%s')",
    user.getName().c_str(),user.getPassword().c_str(),
    user.getState().c_str());

    Mysql mysql;
    if(mysql.connect()){
        if(mysql.update(sql)){
            //获取新插入的数据的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query_id(int id)
{
    char sql[1024];
    sprintf(sql,"select * from user where id=%d",id);

    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setAccount(atoi(row[2]));
                user.setPassword(row[3]);
                user.setState(row[4]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

User UserModel::query_account(int account)
{
    char sql[1024];
    sprintf(sql,"select * from user where account=%d",account);

    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setAccount(atoi(row[2]));
                user.setPassword(row[3]);
                user.setState(row[4]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

User UserModel::query_name(string name)
{
    string sql="select * from user where name="+name;

    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setAccount(atoi(row[2]));
                user.setPassword(row[3]);
                user.setState(row[4]);
                mysql_free_result(res);
                return user;
            }
        }
    }
    return User();
}

bool UserModel::update(User&user)
{
    char sql[1024];
    sprintf(sql,"update user set state ='%s' where account=%d",user.getState().c_str()
    ,user.getAccount());

    Mysql mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}