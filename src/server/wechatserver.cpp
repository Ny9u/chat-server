#include "wechatserver.hpp"
#include "service.hpp"
#include "json.hpp"
#include <functional>
#include <muduo/base/Logging.h>
#include <string>
using namespace std;
using namespace placeholders;
using json=nlohmann::json;

WechatServer::WechatServer(EventLoop *_loop,const InetAddress &listenaddr,
const string &servername):_server(_loop,listenaddr,servername)
{
    //绑定回调函数
    _server.setConnectionCallback(std::bind(&WechatServer::onConnection,this,_1));
    _server.setMessageCallback(std::bind(&WechatServer::onMessage,this,_1,_2,_3));
    //设置工作线程数量
    _server.setThreadNum(4);
}

void WechatServer::start()
{
    _server.start();
}

void WechatServer::onConnection(const TcpConnectionPtr &connection)
{
    if(connection->connected())
    {
        LOG_INFO<<connection->peerAddress().toIpPort()<<" connected to "<<
        connection->localAddress().toIpPort()<<" state: ONLINE";
    }
    else
    {
        //客户端连接断开
        Service::instance()->clientDisconnectException(connection);
        LOG_INFO<<connection->peerAddress().toIpPort()<<" connected to "<<
        connection->localAddress().toIpPort()<<" state: OFFLINE";
        //释放资源
        connection->shutdown();
    }
}

void WechatServer::onMessage(const TcpConnectionPtr &connection,Buffer *buffer,
Timestamp time)
{
    //接收缓冲区中的数据
    string buf=buffer->retrieveAllAsString();
    //数据反序列化
    json js=json::parse(buf);
    //获取业务handler
    auto handler=Service::instance()->getHandler(js["msgid"].get<int>());
    //设置事件处理器,执行相应业务
    handler(connection,js,time);
}
