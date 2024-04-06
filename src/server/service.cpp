#include "servicetype.hpp"
#include "service.hpp"
#include "mysql.hpp"
#include "user.hpp"
#include "group.hpp"
#include <muduo/base/Logging.h>
#include <vector>
using namespace placeholders;

Service::Service(){
    //初始化关系表,添加业务,将业务处理函数和我们设置的变量关联,通过handler使用对应的函数
    handler_map.insert({BIZ_REGIST,std::bind(&Service::regist,this,_1,_2,_3)});
    handler_map.insert({BIZ_LOGIN,std::bind(&Service::login,this,_1,_2,_3)});
    handler_map.insert({BIZ_ONECHAT,std::bind(&Service::onechat,this,_1,_2,_3)});
    handler_map.insert({BIZ_GROUPCHAT,std::bind(&Service::groupchat,this,_1,_2,_3)});
    handler_map.insert({BIZ_ADDFRIEND,std::bind(&Service::addfriend,this,_1,_2,_3)});
    handler_map.insert({BIZ_DELFRIEND,std::bind(&Service::delfriend,this,_1,_2,_3)});
    handler_map.insert({BIZ_CREGROUP,std::bind(&Service::creategroup,this,_1,_2,_3)});
    handler_map.insert({BIZ_JOINGROUP,std::bind(&Service::joingroup,this,_1,_2,_3)});
    handler_map.insert({BIZ_LEFTGROUP,std::bind(&Service::leftgroup,this,_1,_2,_3)});
    handler_map.insert({BIZ_LOGINOUT,std::bind(&Service::loginout,this,_1,_2,_3)});

    if(redis.connect())
    {
        redis.init_notify_handler(std::bind(&Service::SubscribeMessageHandler,this,_1,_2));
    }
}
    
Service *Service:: instance(){
    static Service inst;
    return &inst;
}

Handler Service:: getHandler(int msgid)
{
    auto res=handler_map.find(msgid); 
    if(res!=handler_map.end())
    {
        return handler_map[msgid];
    }
    else
    {
        return [=](const TcpConnectionPtr &connection, json &js, Timestamp time){
            LOG_ERROR<<"HANDLER_ID:"<<msgid<<" can't find the handler";
        };
    }
}

void Service::SubscribeMessageHandler(int account,string msg)
{
    {
        lock_guard<mutex>lock(connect_mutex);
        auto it=connection_map.find(account);
        if(it!=connection_map.end())
        {
            it->second->send(msg);
            return;
        }
        offlinemsgmodel.insert(account,msg);
    }
}

void Service::clientDisconnectException(const TcpConnectionPtr &connection)
{
    User user;
    {
        lock_guard<mutex>lock(connect_mutex);
        for(auto it=connection_map.begin();it!=connection_map.end();it++)
        {
            if(it->second==connection)
            {
                user.setAccount(it->first);
                connection_map.erase(it);
                break;
            }
        }
    }
    redis.unsubscribe(user.getAccount());

    user=usermodel.query_account(user.getAccount());
    user.setState("offline");
    if(usermodel.update(user))
    {
        LOG_INFO<<"IP :"<<connection->peerAddress().toIpPort()<<" user exit!";
    }
    
}

void Service::serverInterruptException()
{
    LOG_ERROR <<" Server interrupt unexpectedly!";
    char sql[1024]="update user set state='offline' where state='online'";
    Mysql mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

void Service::regist(const TcpConnectionPtr &connection, json &js, Timestamp time)
{
    string name=js["name"];
    string password=js["password"];

    User user;
    user.setName(name);
    user.setPassword(password);
    bool state =usermodel.insert(user);
    if(state==true)
    {   
        json respond;
        respond["msgid"]=BIZ_REGIST_ACK;
        respond["errno"]=0;
        respond["useraccount"]=usermodel.query_id(user.getId()).getAccount();
        connection->send(respond.dump());
    }
    else
    {
        json respond;
        respond["msgid"]=BIZ_REGIST_ACK;
        respond["errno"]=1;
        respond["errmsg"]="注册失败,用户名重复!";
        connection->send(respond.dump());
    }
}

void Service:: login(const TcpConnectionPtr &connection, json &js, Timestamp time)
{
    int account=js["account"].get<int>();
    string password=js["password"];

    User user=usermodel.query_account(account);
    if(user.getAccount()==account&&user.getPassword()==password)
    {
        if(user.getState()=="online")
        {
            json respond;
            respond["msgid"]=BIZ_LOGIN_ACK;
            respond["errno"]=1;
            respond["errmsg"]="该账号已登录!";
            connection->send(respond.dump());
        }
        else
        {
            {
                lock_guard<mutex>lock(connect_mutex);
                connection_map.insert({account,connection});
            }
            redis.subscribe(user.getAccount());
            user.setState("online");
            usermodel.update(user);
            json respond;
            respond["msgid"]=BIZ_LOGIN_ACK;
            respond["errno"]=0;
            respond["account"]=user.getAccount();
            respond["name"]=user.getName();
            //检查是否有离线信息
            vector<string> msg=offlinemsgmodel.query(user.getAccount());
            if(!msg.empty())
            {
                respond["offlinemsg"]=msg;
                offlinemsgmodel.remove(user.getAccount());
            }
            //检查好友列表
            vector<User>friendlist=friendmodel.queryFriendlist(account);
            if(!friendlist.empty())
            {
                vector<string>res;
                for(User & fri :friendlist)
                {
                    json js;
                    js["name"]=fri.getName();
                    js["account"]=fri.getAccount();
                    js["state"]=fri.getState();
                    res.push_back(js.dump());
                }
                respond["friend"]= res;
            }
            //检查用户群组
            vector<Group>grouplist=groupmodel.queryUsergroup(account);
            if(!grouplist.empty())
            {
                vector<string>res;
                for(Group & group :grouplist)
                {
                    json js;
                    js["name"]=group.getName();
                    js["groupid"]=group.getId();
                    js["desc"]=group.getDesc();
                    vector<string>v;
                    for(auto &member:group.getMember())
                    {
                        json js;
                        js["account"]=member.getAccount();
                        js["name"]=member.getName();
                        js["state"]=member.getState();
                        js["role"]=member.getRole();
                        v.push_back(js.dump());
                    }
                    js["member"]=v;
                    res.push_back(js.dump());
                }
                respond["group"]= res;
            }
            connection->send(respond.dump());
        }
    }
    else
    {
        json respond;
        respond["msgid"]=BIZ_LOGIN_ACK;
        respond["errno"]=1;
        respond["errmsg"]="登录失败,请检查账号密码是否正确!";
        connection->send(respond.dump());
    }
}

void Service::onechat(const TcpConnectionPtr &connection, json &js, Timestamp time)
{
    int to_account=js["toaccount"].get<int>();
    {
        lock_guard<mutex>lock(connect_mutex);
        auto it=connection_map.find(to_account);
        if(it!=connection_map.end())
        {
            it->second->send(js.dump());
            return;
        }
    }
    
    if (usermodel.query_account(to_account).getState()=="online")
    {
        redis.publish(to_account,js.dump());
        return;
    }
    offlinemsgmodel.insert(to_account,js.dump());

}

void Service::groupchat(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int account=js["account"].get<int>();
    int groupid=js["groupid"].get<int>();
    
    vector<int>member=groupmodel.queryGroupmember(account,groupid);
    lock_guard<mutex>lock(connect_mutex);
    for(auto useraccount:member)
    {  
        auto it=connection_map.find(useraccount);
        if(it!=connection_map.end())
        {
            it->second->send(js.dump());
        }
        else
        {
            if (usermodel.query_account(useraccount).getState()=="online")
            {
                redis.publish(useraccount,js.dump());
            }
            else
            {
                offlinemsgmodel.insert(useraccount,js.dump());
            }
        }
    }
}

void Service:: addfriend(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int useraccount=js["fromaccount"].get<int>();
    int friendaccount=js["toaccount"].get<int>();

    if(friendmodel.insert(useraccount,friendaccount))
    {
        LOG_INFO << useraccount <<" make friend with "<<friendaccount;
    }
}

void Service:: delfriend(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int useraccount=js["fromaccount"].get<int>();
    int friendaccount=js["toaccount"].get<int>();

    if(friendmodel.remove(useraccount,friendaccount))
    {
        LOG_INFO << useraccount <<" delete friend "<<friendaccount;
    }
}

void Service::creategroup(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int account=js["account"].get<int>();
    string name=js["groupname"];
    string desc=js["groupdesc"];

    Group group(name,desc);
    if(groupmodel.createGroup(group))
    {
        LOG_INFO<<"user:"<< account <<" "<<"create group:"<<group.getId();
        groupmodel.joinGroup(account,group.getId(),"creator");
    }
}

void Service::joingroup(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int account=js["account"].get<int>();
    int groupid=js["groupid"].get<int>();
    string role=js["role"];

    if(groupmodel.joinGroup(account,groupid,role))
    {
        LOG_INFO<<"user:"<<account<<"join into group:"<<groupid;
    }
}

void Service::leftgroup(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int account=js["account"].get<int>();
    int groupid=js["groupid"].get<int>();

    if(groupmodel.leftGroup(account,groupid))
    {
        LOG_INFO<<"user:"<<account<<"left the group:"<<groupid;
    }
}

void Service::loginout(const TcpConnectionPtr &connection, json &js,Timestamp time)
{
    int account=js["account"].get<int>();
    {
        lock_guard<mutex>lock(connect_mutex);
        auto it=connection_map.find(account);
        if(it!=connection_map.end())
        {
            connection_map.erase(it);
        }
    }
    /*{
        lock_guard<mutex>lock(connect_mutex);
        for(auto it=connection_map.begin();it!=connection_map.end();it++)
        {
            if(it->second==connection)
            {
                connection_map.erase(it);
                break;
            }
        }
    }*/
    redis.unsubscribe(account);

    User user(account," "," ","offline");
    if(usermodel.update(user))
    {
        LOG_INFO<<"user:"<<account<<" login out";
    }
    
}