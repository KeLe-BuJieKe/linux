#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include "ThreadPool.hpp"
#include "Task.hpp"

static const int DEFAULT = 8081;
static const int BACK_LOG = 5;


class TcpServer
{
  private:
    int port; //端口号
    int listen_sock; //监听套接字
    ThreadPool<Task>* tp;
  public:
    TcpServer(int _port = DEFAULT)
      :port(_port), listen_sock(-1), tp(nullptr)
    {}

    void InitTcpServer()
    {
      listen_sock = socket(AF_INET, SOCK_STREAM, 0);
      if(listen_sock < 0)
      {
        std::cerr << "socket error " << std::endl;
        exit(2);
      }

      struct sockaddr_in local;
      memset(&local, 0, sizeof(local));
      local.sin_family = AF_INET;
      local.sin_port = htons(port);
      local.sin_addr.s_addr = INADDR_ANY;
      socklen_t len = sizeof(local);
      if(bind(listen_sock, reinterpret_cast<struct sockaddr*>(&local), len) < 0)
      {
        std::cerr << "bind error " << std::endl;
        exit(3);
      }
      
      if(listen(listen_sock, BACK_LOG) < 0)
      {
        std::cerr << "listen error " << std::endl;
        exit(4);
      }
      
      tp = new ThreadPool<Task>();
    }

    void Loop()
    {
        tp->InitThreadPool();
        for( ; ; )
        {
            struct sockaddr_in peer;
            socklen_t len = sizeof(peer);
            //真正给你服务得是sock来给你服务，并不是listen_sock
            //listen_sock可以看成一个在外招揽客人的员工
            //每当招揽到人，他就叫其他人(服务员)来招待你，给你提供服务
            int sock = accept(listen_sock, reinterpret_cast<struct sockaddr*>(&peer), &len);
            if(sock < 0) //这里就相当于招揽客人失败
            {
              std::cerr << "accept fail" << std::endl;
              continue;
            }
            
            int _port = ntohs(peer.sin_port);
            std::string _ip = inet_ntoa(peer.sin_addr);
            std::cout << "get a new link ["  << _ip << "]:"<< _port <<std::endl;
            Task t(sock, _ip, _port);
            tp->Push(t);
        }
    }
    

    ~TcpServer()
    {
        if(listen_sock >= 0)
        {
           close(listen_sock);
        }
    }
};

