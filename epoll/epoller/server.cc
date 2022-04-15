#include "reactor.hpp"
#include "util.hpp"
#include "app_interface.hpp"
#include <string>
#include <cstdlib>

const int BACK_LOG = 5;

static void Usage(std::string proc)
{
    std::cout << "Usage: " << "\n\t" << proc << " port" << std::endl; 
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(5);
    }
   
    //这个是与服务器listen_sock相关的
    int port = atoi(argv[1]);
    int listen_sock = ns_sock::Sock::Socket();
    ns_util::SetNotBlock(listen_sock);//ET模式设置非阻塞
    ns_sock::Sock::Bind(listen_sock, port);
    ns_sock::Sock::Listen(listen_sock, BACK_LOG);

    //这个是我们的Epoller事件管理器
    ns_epoll::Epoller epoller;
    epoller.InitEpoller();

    ns_epoll::EventItem item;
    item.R = &epoller;
    item.sock = listen_sock;

    //listen只需要关心读事件就绪
    item.MangerCallBack(ns_appinterface::Accepter, nullptr, nullptr);
    
    epoller.AddEvent(listen_sock, EPOLLIN | EPOLLET, item);

    int timeout = 1000;
    while(true)
    {
        epoller.Dispatcher(timeout);
    }
    return 0;
}
