#pragma once 

#include <iostream>
#include "TcpServer.hpp"
#include "Protocol.hpp"

const int KPORT = 1811;

class HttpServer
{
    private:
        int port;
        TcpServer* tcp_server;
        bool stop;
    public:
        HttpServer(int _port = KPORT)
          :port(_port)
          ,tcp_server(nullptr)
          ,stop(false)
        {}
        
        void InitHttpServer()
        {
            tcp_server = TcpServer::GetInstance(port);
        }
        
        void Loop()
        {
            int listen_sock = tcp_server->GetSock();
            
            while(!this->stop)
            {
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int fd =accept(listen_sock, reinterpret_cast<struct sockaddr*>(&peer), &len); 
                if(fd < 0)
                {
                    continue;
                }

                int *sock = new int(fd);
                pthread_t tid;
                pthread_create(&tid, nullptr, Entrance::HandlerRequest, sock);
            }
        }

        ~HttpServer()
        {}
};


