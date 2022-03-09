#pragma once 

#include <iostream>
#include "TcpServer.hpp"
#include "Protocol.hpp"
#include "Log.hpp"
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
            LOG(INFO, "Loop Begin");
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
                LOG(INFO, "Get a new link ...");
                int *sock = new int(fd);
                pthread_t tid;
                pthread_create(&tid, nullptr, Entrance::HandlerRequest, sock);
            }
        }

        ~HttpServer()
        {}
};


