#include "groupmodel.hpp"
#include "group.hpp"
#include "mysql.hpp"
#include <string>
#include <vector>
using namespace std;

bool GroupModel:: createGroup(Group&group)
{
    string groupname=group.getName();
    string desc=group.getDesc();

    char sql[1024];
    sprintf(sql,"insert into allgroup (groupname,groupdesc) values('%s','%s')",
    groupname.c_str(),desc.c_str());

    Mysql mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

bool GroupModel::joinGroup(int account,int groupid,string role)
{
    char sql[1024];
    sprintf(sql,"insert into groupuser (groupid,useraccount,grouprole) values(%d,%d,'%s')",
    groupid,account,role.c_str());

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

bool GroupModel::leftGroup(int account,int groupid)
{
    char sql[1024];
    sprintf(sql,"delete from groupuser where groupid=%d and useraccount=%d",
    groupid,account);

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
 
vector<Group> GroupModel::queryUsergroup(int account)
{
    char sql[1024];
    sprintf(sql,"select a.groupid,a.groupname,a.groupdesc from allgroup a inner join groupuser b on a.groupid=b.groupid where b.useraccount=%d",
    account);

    vector<Group> grouplist;
    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES*res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                Group g;
                g.setId(atoi(row[0]));
                g.setName(row[1]);
                g.setDesc(row[2]);
                grouplist.push_back(g);
            }
            mysql_free_result(res);
        }
    }
    return grouplist;
}

vector<int> GroupModel::queryGroupmember(int account,int groupid)
{
    char sql[1024];
    sprintf(sql,"select useraccount from groupuser where groupid=%d and useraccount!=%d",
    groupid,account);

    vector<int>groupmember;
    Mysql mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res=mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res))!=nullptr)
            {
                groupmember.push_back(atoi(row[0]));
            }
        }
        mysql_free_result(res);
    }
    return groupmember;
}