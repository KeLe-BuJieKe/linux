#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

static const int DEFAULT = 8081;
static const int BACK_LOG = 5;

class TcpServer
{
  private:
    int port; //端口号
    int listen_sock; //监听套接字
  public:
    TcpServer(int _port = DEFAULT):port(_port),listen_sock(-1)
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

    }

    void Loop()
    {
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
            
            Service(sock, _port, _ip);
        }
    }
    
    //服务
    void Service(int& sock, int port, std::string ip)
    {
        char buffer[1024];
        while(true)
        {
            ssize_t size = read(sock, buffer, sizeof(buffer)-1);
            if(size > 0)
            {
                buffer[size] = 0;
                std::cout << ip << ":" << port  << "# " << buffer << std::endl;
                
                write(sock, buffer, size);
            }
            else if(size == 0)
            {
                std::cout << ip << ":" << port <<" close!!!" << std::endl;
                break;
            }
            else
            {
                std::cerr << sock  << " read error " << std::endl;
                break;
            }
        }
        //关闭文件描述符，相当于服务员已经服务完你之后，那么服务员就去服务其他人了
        //不可能一直再这里服务你
        close(sock);
    }
    ~TcpServer()
    {
        if(listen_sock >= 0)
        {
           close(listen_sock);
        }
    }
};

