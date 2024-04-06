#ifndef REDIS_H
#define REDIS_H

#include<hiredis/hiredis.h>
#include<thread>
#include<functional>
using namespace std;

class Redis
{
public:
    Redis();
    
    ~Redis();

    bool connect();

    bool publish(int channel,string message);

    bool subscribe(int channel);

    bool unsubscribe(int channel);
    //在独立线程中中接收消息
    void observer_channel_message();
    //初始化回调对象
    void init_notify_handler(function<void(int,string)>fn);

private:
    //负责发送的上下文
    redisContext* publish_context;
    //负责接收的上下文
    redisContext* subscribe_context;
    //回调函数,接收到消息时调用
    function<void(int,string)> notify_message_handler;
};


#endif