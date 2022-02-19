#pragma once
#include<iostream>
#include<string>
#include<cstring>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
class UdpClient
{
  public:
    UdpClient(std::string ip, int port):_serverIp(ip),_serverPort(port)
    {}
    bool InitUdpClient()
    {
      _sockfd = socket(AF_INET, SOCK_DGRAM, 0);
      if(this->_sockfd <  0)
      {
        std::cerr << "socket error " << std::endl;
        return false;
      }
      
      //客户端不需要port吗？客户端不需要绑定吗？
      //需要port，但是不需要我们去绑定，系统会自动绑定
      return true;
    }
    void Start()
    {
      struct sockaddr_in peer;
      memset(&peer, 0, sizeof(peer));
      peer.sin_family = AF_INET;
      peer.sin_port = htons(_serverPort);
      peer.sin_addr.s_addr = inet_addr(_serverIp.c_str());
      std::string msg;
      for( ; ;)
      {
        std::cout << "Please Enter#: ";
        std::cin >> msg;
        sendto(this->_sockfd, msg.c_str(), msg.size(), 0, reinterpret_cast<struct sockaddr*>(&peer), sizeof(peer));
      
        char buffer[128];
        struct sockaddr_in temp;
        socklen_t len = sizeof(temp);
        ssize_t size = recvfrom(_sockfd, buffer, sizeof(buffer)-1, 0, reinterpret_cast<struct sockaddr*>(&temp), &len);
        if(size > 0)
        {
          buffer[size] = 0;
          std::cout << buffer << std::endl;
        }
      }
    }
    ~UdpClient()
    {
      if(_sockfd >= 0)
      {
        close(_sockfd);
      }
    }
  private:
    int _sockfd;
    std::string _serverIp;
    int _serverPort;
};
