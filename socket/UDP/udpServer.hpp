#pragma once
#include <iostream>
#include <string>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#define DEFAULT 8081

class UdpServer{
  private:
    //std::string _ip; //去掉
    int _port; //端口号
    int _sockfd;
  public:
    UdpServer(int port = DEFAULT):_port(port),_sockfd(-1)
    {}
    bool InitUdpServer()
    {
      //UDP
      _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      if(_sockfd < 0){
        std::cerr << "socket error " << std::endl;
        return false;
      }
      std::cout << "socket create success, sockfd: " << _sockfd <<std::endl;  //3
      
      struct sockaddr_in local;
      //初始化清空
      memset(&local, '\0', sizeof(local));
      //协议家族
      local.sin_family = AF_INET;
      //把本地序列转换为网络序列
      local.sin_port = htons(_port);
      //将字符串点分十进制ip转换为数字ip
      //local.sin_addr.s_addr = inet_addr(_ip.c_str()); 
      local.sin_addr.s_addr = INADDR_ANY;
      if(bind(_sockfd, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0)
      {
        std::cerr << "bind error " << std::endl;
        return false;
      }
      std::cout << "bind success" <<std::endl;
      return true;
    }
    void Start()
    {
      const size_t SIZE = 128;
      char buffer[SIZE];
      for(; ; )
      {
        struct sockaddr_in peer;
        socklen_t len = sizeof(peer);
        //0 表示以阻塞的方式读取数据
        ssize_t size = recvfrom(_sockfd, buffer, sizeof(buffer)-1, 0, reinterpret_cast<struct sockaddr*>(&peer), &len);
        if(size > 0)
        {
          buffer[size] = '\0';
          //将网络序列转换为主机序列
          int port = ntohs(peer.sin_port);
          //将数字ip转化为点分十进制字符串ip
          std::string ip = inet_ntoa(peer.sin_addr);
          std::cout << ip << ":" << port << "#: " << buffer << std::endl;
        
          //把收到的消息又给用户发回去了
          std::string echo_msg = "server get!->";
          echo_msg += buffer;
          sendto(_sockfd, echo_msg.c_str(), echo_msg.size(), 0, reinterpret_cast<struct sockaddr*>(&peer), len);
        }
        else
        {
          std::cerr << "recvfrom error " << std::endl;
        }
      }
    }
    ~UdpServer()
    {
      if(_sockfd >= 0)
      {
        close(_sockfd);
      }
    }
};
