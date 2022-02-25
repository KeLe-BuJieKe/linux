#pragma once

#include <iostream>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>


class TcpClient
{
    private:
      std::string server_ip; //服务器ip
      int server_port;   //服务器端口号
      int sock;
    public:
      TcpClient(std::string _ip, int port):server_ip(_ip),server_port(port),sock(-1)
      {}
      
      void InitTcpClient()
      {
          sock = socket(AF_INET, SOCK_STREAM, 0);
          if(sock < 0)
          {
              std::cerr << "socket error " << std::endl;
              exit(2);
          }
      }
      
      void Start()
      {
          struct sockaddr_in peer;
          memset(&peer, 0, sizeof(peer));
          peer.sin_family = AF_INET;
          peer.sin_port = htons(server_port);
          peer.sin_addr.s_addr = inet_addr(server_ip.c_str());
          if(connect(sock, reinterpret_cast<struct sockaddr*>(&peer), sizeof(peer)) == 0)
          {
              //success
              std::cout << "connect success ..." << std::endl;
              Request(sock);  
          }
          else
          {
              //fail
            std::cerr << "connect failed ..." << std::endl;
          }
      }

      void Request(int _sock)
      {
          char buffer[1024];
          std::string message;
          while(true)
          {
              std::cout << "Please Enter #";
              std::cin >> message;

              write(_sock, message.c_str(), message.size());
              ssize_t size = read(_sock, buffer, sizeof(buffer)-1);
              if(size > 0)
              {
                  buffer[size] = 0;
                  std::cout << "server echp# " << buffer << std::endl;
              }
          }
      }

      ~TcpClient()
      {
          if(sock >= 0)
          {
              close(sock);
          }
      }
};
