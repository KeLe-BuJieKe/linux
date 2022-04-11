#pragma once 
#include <iostream>
#include "epoller.hpp"
#include "util.hpp"

namespace ns_appinterface{
    using namespace ns_epoll;
    int Recver(EventItem* item)
    {
        //负责数据读取
        return 0;
    }

    int Sender(EventItem* item)
    {
        //负责数据发送
        return 0;
    }

    int Errorer(EventItem* item)
    {
        if (item->sock >= 0)
        {
          close(item->sock);
        }
        return 0;
    }

    //listen_sock获取链接
    int Accepter(EventItem* item)
    {
        std::cout << "get a new link: " << item->sock << std::endl;
        while (true){
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            int sock = accept(item->sock, reinterpret_cast<struct sockaddr*>(&peer), &len);
            if (sock < 0){
                if (errno == EAGAIN || errno == EWOULDBLOCK){
                    //说明没有读取出错，只是底层还没有链接到来
                    return 0;
                }

                if (errno == EINTR){
                    //说明读取的过程中被信号打断了
                    continue;
                }
                else {
                    //真正的出错了
                    return -1;
                }
            }
            else{
                //accept成功，首先先设置非阻塞
                ns_util::SetNotBlock(sock);
                EventItem temp;
                temp.sock = sock;
                temp.R = item->R;
                temp.MangerCallBack(Recver, Sender, Errorer);
                Epoller* epoller = item->R;
                epoller->AddEvent(sock, EPOLLIN | EPOLLET, temp);
            }
        }
        return 0;
    }
}
