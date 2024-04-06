#include<iostream>
#include<string>
#include<vector>
#include<unordered_map>
#include<ctime>
#include<chrono>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<thread>
#include<semaphore.h>
#include<atomic>

#include"json.hpp"
#include"groupuser.hpp"
#include"group.hpp"
#include"user.hpp"
#include"servicetype.hpp"

using json=nlohmann::json;

vector<User>currentUserFriendlist;
vector<Group>currentUserGrouplist;
User currentUser;
//客户端是否在运行
bool ismainmenurunning=false;
//是否有用户登陆
atomic_bool isloginin{false};
//信号量
sem_t sem;

void help(int clientfd=0,string str="");
void onechat(int ,string);
void addfriend(int,string);
void creategroup(int,string);
void joingroup(int,string);
void groupchat(int,string);
void quit(int,string);
unordered_map<string,string>commandMap={
    {"help","显示所有支持的命令,格式help"},
    {"onechat","一对一聊天,格式onechat:friendaccount:message"},
    {"addfriend","添加好友,格式addfriend:friendaccount"},
    {"creategroup","创建群组,格式creategroup:groupname:groupdesc"},
    {"joingroup","添加群组,格式joingroup:groupid"},
    {"groupchat","群聊,格式groupchat:groupid:message"},
    {"quit","退出当前账号,格式quit"}
};

unordered_map<string,function<void(int,string)>>commandHandlerMap={
    {"help",help},
    {"onechat",onechat},
    {"addfriend",addfriend},
    {"creategroup",creategroup},
    {"joingroup",joingroup},
    {"groupchat",groupchat},
    {"quit",quit}
};
string getCurrentTime()
{
    auto tt=std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm*ptm=localtime(&tt);
    char date[60];
    sprintf(date,"%d-%02d-%02d %02d:%02d:%02d",
    (int)ptm->tm_year+1900,(int)ptm->tm_mon+1,(int)ptm->tm_mday,
    (int)ptm->tm_hour,(int)ptm->tm_min,(int)ptm->tm_sec);
    return std::string(date);
}

void showCurrentUserData()
{
    cout<<"------------login user------------"<<endl;
    cout<<"current login user--->account:"<<currentUser.getAccount()<<" name:"<<currentUser.getName()<<endl;
    
    cout<<"------------friend list------------"<<endl;
    if (!currentUserFriendlist.empty())
    {
        for (User&user:currentUserFriendlist)
        {
            cout<<user.getAccount()<<" "<<user.getName()<<" "<<user.getState()<<endl;
        }
    }
    
    cout<<"------------group list------------"<<endl;
    if (!currentUserGrouplist.empty())
    {
        for (Group&group:currentUserGrouplist)
        {
            cout<<group.getName()<<" "<<group.getDesc()<<endl;
            cout<<"群组成员:";
            for (GroupUser &member:(group.getMember()))
            {
                cout<<member.getName()<<" ";
            }
            cout<<endl;
        }
    }

    cout<<"----------------------------------"<<endl;
}

void registResponse(json &response)
{
    if(response["errno"].get<int>()!=0)
    {
        cerr<<response["errmsg"]<<endl;
    }
    else
    {
        cout<<"注册成功,你的账号为:"<<response["useraccount"].get<int>()<<endl;
    }
}
void loginResponse(json&response)
{
    if(response["errno"].get<int>()!=0)
    {
        cerr<<response["errmsg"]<<endl;
    }
    else
    {
        currentUser.setAccount(response["account"].get<int>());
        currentUser.setName(response["name"]);
        
        if (response.contains("friend"))
        {
            currentUserFriendlist.clear();

            vector<string>friendlist=response["friend"];
            for(string &str:friendlist)
            {
                json js=json::parse(str);
                User user;
                user.setAccount(js["account"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                currentUserFriendlist.push_back(user);
            }
        }

        if (response.contains("group"))
        {   
            currentUserGrouplist.clear();

            vector<string>grouplist=response["group"];
            for(string &str:grouplist)
            {
                json js=json::parse(str);
                Group group;
                group.setId(js["groupid"].get<int>());
                group.setName(js["name"]);
                group.setDesc(js["desc"]);
                
                vector<string>memberlist=response["member"];
                for(string &member:memberlist)
                {
                    GroupUser user;
                    json js=json::parse(member);
                    user.setAccount(js["account"].get<int>());
                    user.setName(js["name"]);
                    user.setState(js["state"]);
                    user.setRole(js["role"]);
                    group.getMember().push_back(user);
                }
                currentUserGrouplist.push_back(group);
            }
        }
        showCurrentUserData();

        if (response.contains("offlinemsg"))
        {
            vector<string>v=response["offlinemsg"];
            for(string &str:v)
            {
                json js=json::parse(str);
                if(js["msgid"].get<int>()==BIZ_ONECHAT)
                {
                    cout<<js["time"].get<string>()<<js["name"].get<string>()<<"("
                    <<js["account"].get<int>()<<")"<<":"<<js["msg"].get<string>()<<endl;
                }
                if(js["msgid"].get<int>()==BIZ_GROUPCHAT)
                {
                    cout<<"群消息"<<js["time"].get<string>()<<js["groupid"].get<int>()
                    <<":"<<js["msg"].get<string>()<<endl;
                }
            }
        }
        isloginin=true;
    }
}

void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024]={0};
        int len=recv(clientfd,buffer,1024,0);
        if(len==-1||len==0)
        {
            close(clientfd);
            exit(-1);
        }

        json js=json::parse(buffer);
        if(js["msgid"].get<int>()==BIZ_ONECHAT)
        {
            /*cout<<js["time"].get<string>()<<js["name"].get<string>()<<"("
            <<js["account"].get<int>()<<")"<<":"<<js["msg"].get<string>()<<endl;*/
            cout<<js["msg"].get<string>()<<endl;
            continue;
        }
        
        if(js["msgid"].get<int>()==BIZ_GROUPCHAT)
        {
            /*cout<<"群消息"<<js["time"].get<string>()<<js["groupid"].get<int>()
            <<":"<<js["msg"].get<string>()<<endl;*/
            cout<<js["msg"].get<string>()<<endl;
            continue;
        }

        if(js["msgid"].get<int>()==BIZ_REGIST_ACK)
        {
            registResponse(js);
            sem_post(&sem);
            continue;
        }

        if(js["msgid"].get<int>()==BIZ_LOGIN_ACK)
        {
            loginResponse(js);
            sem_post(&sem);
            continue;
        }

    }
}

void mainMenu(int clientfd)
{
    help();

    char buffer[1024]={0};
    while(ismainmenurunning)
    {
        cin.getline(buffer,1024);
        string commandbuf(buffer);
        string command;
        int index=commandbuf.find(":");
        if(index==-1)
        {
            command=commandbuf;
        }
        else
        {
            command=commandbuf.substr(0,index);
        }
        auto it=commandHandlerMap.find(command);
        if(it==commandHandlerMap.end())
        {
            cout<<"命令格式错误"<<endl;
            continue;
        }
        else
        {
            it->second(clientfd,commandbuf.substr(index+1,commandbuf.size()-index));
        }
    }
}

void help(int clientfd,string str)//功能正常
{
    cout<<"------------命令菜单------------"<<endl;
    for(auto &c:commandMap)
    {
        cout<<c.first<<":"<<c.second<<endl;
    }
    cout<<endl;
}
//json太长会导致崩溃,中文编码问题已解决,原因是mysql-server字符集未设置为utf8
void onechat(int clientfd,string str) //功能正常,三种方式都已验证
{
    int index=str.find(":");
    if(index==-1)
    {
        cerr<<"消息发送失败"<<endl;
        return;
    }
    
    int friendaccount=atoi(str.substr(0,index).c_str());
    string message=str.substr(index+1,str.size()-index);

    json js; 
    js["msgid"]=BIZ_ONECHAT;
    js["toaccount"]=friendaccount;
    js["msg"]=message;
    //js["account"]=currentUser.getAccount();
    //js["name"]=currentUser.getName();
    //js["time"]=getCurrentTime();
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"消息发送失败"<<endl;
    }
    else
    {
        cout<<currentUser.getName()<<":"<<message<<endl;
    }
}

void addfriend(int clientfd,string str)//功能正常
{
    int friendaccount=atoi(str.c_str());
    json js;
    js["msgid"]=BIZ_ADDFRIEND;
    js["fromaccount"]=currentUser.getAccount();
    js["toaccount"]=friendaccount;
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"添加好友失败"<<endl;
    }
    else
    {
        cout<<"好友添加成功"<<endl;
    }
}

void creategroup(int client,string str)//功能正常
{
    int index=str.find(":");
    if(index==-1)
    {
        cerr<<"创建群组失败"<<endl;
        return;
    }

    string groupname=str.substr(0,index);
    string groupdesc=str.substr(index+1,str.size()-index);

    json js;
    js["msgid"]=BIZ_CREGROUP;
    js["account"]=currentUser.getAccount();
    js["groupname"]=groupname;
    js["groupdesc"]=groupdesc;
    string buffer=js.dump();

    int len=send(client,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"创建群组失败"<<endl;
    }
    else
    {
        cout<<"创建群组成功"<<endl;
    }
}

void joingroup(int clientfd,string str)//功能正常
{
    int groupid=atoi(str.c_str());

    json js;
    js["msgid"]=BIZ_JOINGROUP;
    js["groupid"]=groupid;
    js["account"]=currentUser.getAccount();
    js["role"]="normal";
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"加入群组失败"<<endl;
    }
    else
    {
        cerr<<"加入群组成功"<<endl;
    }
}

void groupchat(int clientfd,string str) //功能正常
{
    int index=str.find(":");
    if(index==-1)
    {
        cerr<<"消息发送失败"<<endl;
        return;
    }

    int groupid=atoi(str.substr(0,index).c_str());
    string message=str.substr(index+1,str.size()-index);

    json js;
    js["msgid"]=BIZ_GROUPCHAT;
    js["account"]=currentUser.getAccount();
    js["name"]=currentUser.getName();
    js["groupid"]=groupid;
    js["msg"]=message;
    js["time"]=getCurrentTime();
    string buffer =js.dump();
    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"消息发送失败"<<endl;
    }
}
//quit之后重新登陆会出现阻塞,账号无法再次登录,原因是读线程读取了登录响应报文但无处理
void quit(int clientfd,string str)
{
    json js;
    js["msgid"]=BIZ_LOGINOUT;
    js["account"]=currentUser.getAccount();
    string buffer=js.dump();

    int len=send(clientfd,buffer.c_str(),strlen(buffer.c_str())+1,0);
    if(len==-1)
    {
        cerr<<"退出失败"<<endl;
    }else
    {
        ismainmenurunning=false;
    }
}

int main(int agrc,char**argv)
{ 
    if(agrc<3)
    {
        cerr<<"command invalid emample: ./client 127.0.0.1 6000"<<endl;
        exit(-1);
    }

    char *ip=argv[1];
    uint16_t port=atoi(argv[2]);

    int clientfd=socket(AF_INET,SOCK_STREAM,0);
    if(clientfd==-1)
    {
        cerr<<"socket create fail"<<endl;
        exit(-1);
    }

    sockaddr_in server;
    memset(&server,0,sizeof(sockaddr_in));

    server.sin_family=AF_INET;//设置协议族
    server.sin_port=htons(port);//设置通信端口
    server.sin_addr.s_addr=inet_addr(ip);//设置服务器ip

    if(connect(clientfd,(sockaddr*)&server,sizeof(sockaddr_in))==-1)
    {
        cerr<<"connect server fail"<<endl;
        close(clientfd);
        exit(-1);
    }
    //初始化信号量
    sem_init(&sem,0,0);
    //启动接收线程
    std::thread readTask(readTaskHandler,clientfd);
    readTask.detach();

    for(;;)
    {
        ismainmenurunning=false;
        cout<<"------------功能菜单------------"<<endl;
        cout<<"1.login"<<endl;
        cout<<"2.regist"<<endl;
        cout<<"3.exit"<<endl;
        cout<<"-------------------------------"<<endl;

        int choice=0;
        cout<<"choice:";
        cin>>choice;
        cin.get();

        switch(choice)
        {          
            case 1://出现json解析失败的问题,原因是json解析到不存在的字段
            {   
                int account;
                char password[50]={0};
                cout<<"account:";cin>>account;
                cin.get();//读掉残存的回车
                cout<<"password:";
                cin.getline(password,50);

                json js;
                js["msgid"]=BIZ_LOGIN;
                js["account"]=account;
                js["password"]=password;
                string request=js.dump();

                int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len==-1)
                {
                    cerr<<"send login msg fail"<<endl;
                }
               
                sem_wait(&sem);
                if (isloginin)
                {
                    ismainmenurunning=true;
                    mainMenu(clientfd);
                }
                break;
            }

            case 2://功能正常
            {
                char name[50];
                char password[50];
                cout<<"username:";
                cin.getline(name,50);
                cout<<"password:";
                cin.getline(password,50);

                json js;
                js["msgid"]=BIZ_REGIST;
                js["name"]=name;
                js["password"]=password;
                string request=js.dump();

                int len=send(clientfd,request.c_str(),strlen(request.c_str())+1,0);
                if(len==1)
                {
                    cerr<<"send regist msg fail"<<request<<endl;
                }
                sem_wait(&sem);
                
                break;
            }

            case 3://功能正常
            {
                close(clientfd);
                sem_destroy(&sem);
                exit(0);
            }

            default:
            {
                cerr<<"invalid input!"<<endl;
                break;
            }

        }
    }

}

