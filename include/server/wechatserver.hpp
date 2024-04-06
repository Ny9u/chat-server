#ifndef WECHATSERVER_H
#define WECHATSERVER_H

#include "json.hpp"
#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <iostream>
#include <functional>
#include <string>

using namespace std;
using namespace muduo;
using namespace muduo::net;
using namespace placeholders;
using json=nlohmann::json;

class WechatServer{
public:
    //组合tcpserver对象
    WechatServer(EventLoop *_loop,const InetAddress &listenaddr,const string &servername);
    //开启服务
    void start();
private:
    //回调函数
    void onConnection(const TcpConnectionPtr &connection);
    
    void onMessage(const TcpConnectionPtr &connection,Buffer *buffer,
Timestamp time);
    //服务器对象
    TcpServer _server;
    
    EventLoop *_loop;
};
#endif