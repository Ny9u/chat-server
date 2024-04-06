#ifndef SERVICE_H
#define SERVICE_H

#include"json.hpp"
#include"usermodel.hpp"
#include"offlinemessagemodel.hpp"
#include"friendmodel.hpp"
#include"groupmodel.hpp"
#include"redis.hpp"
#include<muduo/net/TcpServer.h>
#include<unordered_map>
#include<mutex>
#include<functional>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using json = nlohmann::json;
//定义业务处理类型(函数指针)
using Handler= std::function<void(const TcpConnectionPtr&connection, json &js, Timestamp time)>;
//typedef void (*Handler)(const TcpConnectionPtr &connection, json &js, Timestamp&time);

class Service{
public:
    //获取单例对象,外部通过这个单例对象调用业务
    static Service * instance();
    //获取对应业务handler
    Handler getHandler(int id);
    //获取订阅通道消息handler
    void SubscribeMessageHandler(int id,string msg);
    //客户端异常退出
    void clientDisconnectException(const TcpConnectionPtr &connection);
    //服务器异常退出
    void serverInterruptException();
    //注册业务
    void regist(const TcpConnectionPtr &connection, json &js, Timestamp time);
    //登录业务
    void login(const TcpConnectionPtr &connection, json &js, Timestamp time);
    //单人聊天
    void onechat(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //群组聊天
    void groupchat(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //添加好友
    void addfriend(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //删除好友
    void delfriend(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //创建群组
    void creategroup(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //加入群组
    void joingroup(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //退出群组
    void leftgroup(const TcpConnectionPtr &connection, json &js,Timestamp time);
    //退出账号
    void loginout(const TcpConnectionPtr &connection, json &js,Timestamp time);
private:
    //私有,只能单例使用
    Service();
    //存储业务对应关系
    unordered_map<int,Handler> handler_map; 
    //存储用户在线连接
    unordered_map<int,TcpConnectionPtr>connection_map;
    //互斥锁,保证用户在线表安全
    mutex connect_mutex;
    //mysql数据库操作对象
    UserModel usermodel;
    OfflinemessageModel offlinemsgmodel;
    FriendModel friendmodel;
    GroupModel groupmodel;
    //redis数据库操作对象
    Redis redis;
};

#endif