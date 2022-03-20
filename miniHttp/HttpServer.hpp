#pragma once 

#include <iostream>
#include <signal.h>
#include "TcpServer.hpp"
#include "Log.hpp"
#include "Task.hpp"
#include "ThreadPool.hpp"
const int KPORT = 1811;

class HttpServer
{
    private:
        int port;
        bool stop;
    public:
        HttpServer(int _port = KPORT)
          :port(_port)
          ,stop(false)
        {}
        
        void InitHttpServer()
        {
            //tcp_server = TcpServer::GetInstance(port);
            //将SIGPIPE信号进行忽略，如果不忽略在写入时，可能直接奔溃server
            signal(SIGPIPE, SIG_IGN);
        }
        
        void Loop()
        {
            LOG(INFO, "Loop Begin");
            TcpServer* svr = TcpServer::GetInstance(port);
            int listen_sock = svr->GetSock();
            ThreadPool* thread_pool = ThreadPool::GetInstance();
            while(!stop)
            {
                struct sockaddr_in peer;
                socklen_t len = sizeof(peer);
                int fd = accept(listen_sock, reinterpret_cast<struct sockaddr*>(&peer), &len); 
                if(fd < 0)
                {
                    continue;
                }
                LOG(INFO, "Get a new link ...");
                
                Task task(fd);
                thread_pool->PushTask(task);
            }
        }

        ~HttpServer()
        {}
};


