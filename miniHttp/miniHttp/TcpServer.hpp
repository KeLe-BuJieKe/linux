#pragma once
#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 
#include <pthread.h>

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
            CreateSocket();
            BindSocket();
            ListenSocket();
        }

        void CreateSocket()  //创建监听套接字
        {
            listen_sock = socket(AF_INET, SOCK_STREAM, 0);  
            if(listen_sock < 0)
            {
                exit(2);
            }
            
            int opt = 1;
            setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
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
                exit(3);
            }
        }

        void ListenSocket() //将监听套接字设置为listen状态
        {
              int ret = listen(listen_sock, KBACKLOG);
              if(ret < 0)
              {
                  exit(4);
              }
        }
        
        int GetSock()
        {
            return listen_sock;
        }


        ~TcpServer()
        {

        }
};


TcpServer* TcpServer::only_svr = nullptr;
