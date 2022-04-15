#pragma once 
#include <iostream>
#include <string>
#include "reactor.hpp"
#include "util.hpp"

namespace ns_appinterface{
    using namespace ns_epoll;
    
    //0 :  读取成功
    //-1： 读取失败
    int RecverHelper(int sock, std::string* out)
    {
        while (true){
            char buffer[128];
            ssize_t size = recv(sock, buffer, sizeof(buffer) - 1, 0);

            if (size < 0){
                if (errno == EAGAIN || errno == EWOULDBLOCK)
                {
                    //循环读取，将数据已经读取完毕了
                    return 0;
                }
                else if (errno == EINTR) 
                {
                    //被信号中断，就让他继续读取
                    continue;
                }
                else 
                {
                    //真正读取出错了，
                    return -1;
                }
            }
            else{
                buffer[size] = '\0';
                //将我们读取到的内容，放入到该文件描述符所对应的事件输入缓冲区中
                *out += buffer;
            }
        }
    }

    int Recver(EventItem* item)
    {
        //负责数据读取
        std::cout << "recv event ready: " << item->sock << std::endl; 

        //1.由于我们的epoll是ET模式，非阻塞，所以这就导致着我们必须一次性读完
        if (RecverHelper(item->sock, &item->inbuffer) < 0){
            item->error_handler(item);
            return -1;
        }

        std::cout << "client# " << item->inbuffer << std::endl;
        //2.根据发送过来的数据流，进行包与包之间的分离，防止粘包问题，这里是涉及协议定制
        std::vector<std::string> message;
        ns_util::StringUtil::Spilt(item->inbuffer, message, "X");
        
        //3.针对一个个得报文协议反序列化
        int x = 0;
        int y = 0;
        for (std::string& str : message){
            ns_util::StringUtil::Deserialize(str, &x, &y);
            
            //4.处理业务
            int z = x + y;

            //5.形成响应报文，序列化转换为一个字符串
            std::string response;
            response += std::to_string(x);
            response += "+";
            response += std::to_string(y);
            response += "=";
            response += std::to_string(z);
            item->outbuffer += response;

            //5.1设置响应报文和响应报文之间得分隔符 序列号
            item->outbuffer += "X\n"; //encode
        }

        //6.写回 让epoll关注读就绪
        if (!item->outbuffer.empty()){
            item->R->EnableReadWrite(item->sock, true, true);
        }

        return 0;
    }


    //0: 写完outbuffer
    //1：缓冲区打满，下次写入
    //-1:写出错
    int SenderHelper(int sock, std::string& out)
    {
        size_t total = 0;
        while (true){
          ssize_t len = send(sock, out.c_str()+total, out.size()-total, 0);
          if (len > 0)
          {
              total += len;
              if (total >= out.size())
              {
                out.erase(0);
                return 0;
              }
          }
          else if (len < 0)
          {
             if (errno == EAGAIN || errno == EWOULDBLOCK)
             {
               //无论是否发送完，outbuffer,都需要将已经发送得数据，全部移出缓冲区
               out.erase(0, total); //已经将缓冲区打满，不能在写入了，但是并不一定写完
               return 1;
             }
             else if (errno == EINTR)
             {
                continue;
             }
             else 
             {
               return -1;
             }
          }
        }
    }

    int Sender(EventItem* item)
    {
        //负责数据发送
        int ret = SenderHelper(item->sock, item->outbuffer);
        if (ret == 0)
        {
          item->R->EnableReadWrite(item->sock, true, false);
        }
        else if (ret == 1)
        {
          //默认打开了
          
          item->R->EnableReadWrite(item->sock, true, true);
        }
        else 
        {
          item->error_handler(item);
        }
        
        return 0;
    }

    int Errorer(EventItem* item)
    {
        close(item->sock);
        item->R->DelEvent(item->sock);
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
                //epoll,经常一定会设置读事件就绪，而写事件我们是按需打开的
                epoller->AddEvent(sock, EPOLLIN | EPOLLET, temp);
            }
        }
        return 0;
    }
}
