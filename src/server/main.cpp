#include "wechatserver.hpp"
#include "service.hpp"
#include <signal.h>
void interruptException(int)
{
    Service::instance()->serverInterruptException();
    exit(0);
}

int main(int argc,char**argv)
{
    if(argc<3)
    {
        cerr<<"command invaild! example: ./server 127.0.0.1 6000"<<endl;
        exit(-1);
    }
    
    char *ip=argv[1];
    uint16_t port=atoi(argv[2]);

    signal(SIGINT,interruptException);
    EventLoop loop; 
    InetAddress addr(ip,port);
    WechatServer server(&loop,addr,"server");

    server.start();
    loop.loop();
    return 0;
}