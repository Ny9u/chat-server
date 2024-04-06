#include "mysql.hpp"
#include "offlinemessagemodel.hpp"
using namespace std;

bool OfflinemessageModel:: insert(int account,string msg)
{
    char sql[1024];
    sprintf(sql,"insert into offlinemessage (account,message) values (%d,'%s')",
    account,msg.c_str());

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

vector<string> OfflinemessageModel:: query(int account)
{
    char sql[1024];
    sprintf(sql,"select message from offlinemessage where account=%d",account);

    Mysql mysql;
    vector<string>v;
    if(mysql.connect())
    {
        MYSQL_RES *res=mysql.query(sql);
        if(res!=nullptr)
        {        
            MYSQL_ROW row=mysql_fetch_row(res);
            if(row!=nullptr)
            {
               v.push_back(row[0]);
               mysql_free_result(res);
            }
        }
    }
    return v;
}
            
bool OfflinemessageModel::remove(int account)
{
    char sql[1024];
    sprintf(sql,"delete from offlinemessage where account=%d",account);

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