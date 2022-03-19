#pragma once
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <pthread.h>
#include <signal.h>
#include "Log.hpp"

const int KBACKLOG = 5;


class TcpServer
{
    private:
        int port;
        int listen_sock;
        static TcpServer* only_svr;
    
    private: 
        TcpServer(int _port)
          :port(_port)
          ,listen_sock(-1)
        {}
        
        TcpServer(const TcpServer&) = delete;
        TcpServer& operator=(const TcpServer&) = delete;
    public:
        static TcpServer* GetInstance(int port)
        {
            static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
            
            if(nullptr == only_svr)
            {
                pthread_mutex_lock(&lock);
                if(nullptr == only_svr)
                {
                    only_svr = new TcpServer(port);
                    only_svr->InitTcpServer();
                }
                pthread_mutex_unlock(&lock);
            }

            return only_svr;
        }
        
        void InitTcpServer()  //初始化服务器
        {
            //将SIGPIPE信号进行忽略，如果不忽略在写入时，可能直接奔溃server
            signal(SIGPIPE, SIG_IGN);
            CreateSocket();
            BindSocket();
            ListenSocket();
            LOG(INFO, "tcp server init success ...");
        }

        void CreateSocket()  //创建监听套接字
        {
            listen_sock = socket(AF_INET, SOCK_STREAM, 0);  
            if(listen_sock < 0)
            {
                LOG(FATAL, "socket error");
                exit(2);
            }
            
            int opt = 1;
            setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
            LOG(INFO, "create socket success ...");
        }
        
        void BindSocket() //绑定
        {
            struct sockaddr_in local;
            memset(&local, 0, sizeof(local));
            local.sin_family = AF_INET;
            local.sin_port = htons(port);
            local.sin_addr.s_addr = INADDR_ANY;
            socklen_t len = sizeof(local);

            int ret = bind(listen_sock, reinterpret_cast<struct sockaddr*>(&local), len);
            if(ret < 0)
            {
                LOG(FATAL, "bind error");
                exit(3);
            }
            LOG(INFO, "bind socket success ...");
        }

        void ListenSocket() //将监听套接字设置为listen状态
        {
              int ret = listen(listen_sock, KBACKLOG);
              if(ret < 0)
              {
                  LOG(FATAL, "listen error");
                  exit(4);
              }
              LOG(INFO, "listen socket success ...");
        }
        
        int GetSock()
        {
            return listen_sock;
        }


        ~TcpServer()
        {
            if(listen_sock >= 0)
            {
                close(listen_sock);
            }
        }
};


TcpServer* TcpServer::only_svr = nullptr;
