#include "redis.hpp"
#include <thread>
#include <muduo/base/Logging.h>
#include <iostream>
using namespace std;

Redis::Redis():publish_context(nullptr),subscribe_context(nullptr)
{

}

Redis::~Redis()
{
    if(publish_context!=nullptr)
    {
        redisFree(publish_context);
    }
    if(subscribe_context!=nullptr)
    {
        redisFree(subscribe_context);
    }
}

void Redis::observer_channel_message()//json字段不匹配会导致这里出问题
{
    LOG_INFO<<"observer_channel_message is working";
    redisReply*reply=nullptr;
    while(REDIS_OK==redisGetReply(this->subscribe_context,(void**)&reply))
    {
        if(reply!=nullptr&&reply->element!=nullptr&&reply->element[2]!=nullptr&&reply->element[2]->str!=nullptr)
        {
            LOG_INFO<<"observer message: "<<reply->element[2]->str;
            notify_message_handler(atoi(reply->element[1]->str),reply->element[2]->str);
        }
        freeReplyObject(reply); 
    }
    LOG_ERROR<<"observer_channel_message quit";
}

bool Redis::connect()
{
    publish_context=redisConnect("127.0.0.1",6379);
    if(publish_context==nullptr)
    {
        LOG_ERROR<<"connect redis fail";
        return false;
    }

    subscribe_context=redisConnect("127.0.0.1",6379);
    if(subscribe_context==nullptr)
    {
        LOG_ERROR<<"connect redis fail";
        return false;
    }

    string password="your password";
    redisReply*reply_p=(redisReply*)redisCommand(publish_context,"AUTH %s",
    password.c_str());
    if(reply_p==nullptr)
    {
        LOG_ERROR<<"connect redis fail";
        return false;
    }
    redisReply*reply_s=(redisReply*)redisCommand(subscribe_context,"AUTH %s",
    password.c_str());
    if(reply_s==nullptr)
    {
        LOG_ERROR<<"connect redis fail";
        return false;
    }

    thread t([&](){
        //在独立线程中调用函数专门接收消息
        observer_channel_message();
    });
    t.detach();
    
    LOG_INFO<<"connect to redis-server";
    return true;
}

bool Redis::publish(int channel,string message)
{
    redisReply*reply=(redisReply*)redisCommand(publish_context,"PUBLISH %d %s",
    channel,message.c_str());
    if(reply==nullptr)
    {
        LOG_ERROR<<"publish message fail";
        return false;
    }
    LOG_INFO<<"publish message: "<<message;
    freeReplyObject(reply);
    return true;
}

bool Redis::subscribe(int channel)
{
    if(REDIS_ERR==redisAppendCommand(subscribe_context,"SUBSCRIBE %d",channel))
    {
        LOG_ERROR<<"subscribe channel fail";
        return false;
    }
    int done=0;
    while(!done)
    {
        if(REDIS_ERR==redisBufferWrite(subscribe_context,&done))
        {
            LOG_ERROR<<"subscribe channel fail";
            return false;
        }
    }
    LOG_INFO<<"subscribe channel: "<<channel;
    return true;
} 

bool Redis::unsubscribe(int channel)
{
    if(REDIS_ERR==redisAppendCommand(subscribe_context,"UNSUBSCRIBE %d",channel))
    {
        LOG_ERROR<<"unsubscribe channel fail";
        return false;
    }
    int done=0;
    while(!done)
    {
        if(REDIS_ERR==redisBufferWrite(subscribe_context,&done))
        {
            LOG_ERROR<<"unsubscribe channel fail";
            return false;
        }
    }
    LOG_INFO<<"unsubscribe channel: "<<channel;
    return true;
} 
 void Redis::init_notify_handler(function<void(int,string)>fn)
{
    //设置回调函数
    this->notify_message_handler=fn;
}