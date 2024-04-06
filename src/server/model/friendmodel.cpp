#include "friendmodel.hpp"
#include "mysql.hpp"
using namespace std;

bool FriendModel:: insert(int usercnt,int friendcnt)
{
    char pre_sql[1024];
    sprintf(pre_sql,"select * from friend where usercnt=%d and friendcnt=%d",
    usercnt,friendcnt);
    
    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res=mysql.query(pre_sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row==nullptr)
            {
                char sql[128];
                sprintf(sql,"insert into friend (usercnt,friendcnt) values(%d,%d)",
                usercnt,friendcnt);
                if(mysql.update(sql))
                {
                    mysql_free_result(res);
                    return true;
                }
            }
        }
        mysql_free_result(res);
    }
    return false;
}

vector<User> FriendModel:: queryFriendlist(int account)
{
    char sql[1024];
    sprintf(sql,"select a.name,a.account,a.state from user a inner join friend b on b.friendcnt=a.account where b.usercnt=%d",
    account);

    Mysql mysql;
    vector<User>friendlist;
    if(mysql.connect())
    {
        MYSQL_RES *res=mysql.query(sql);
        if (res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setName(row[0]);
                user.setAccount(atoi(row[1]));
                user.setState(row[2]);
                friendlist.push_back(user);
            }
            mysql_free_result(res);
        }
        
    }
    return friendlist;
}

bool FriendModel::remove(int usercnt,int friendcnt)
{
    char sql[1024];
    sprintf(sql,"delete from friend where usercnt=%d and friendcnt=%d",
    usercnt,friendcnt);

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