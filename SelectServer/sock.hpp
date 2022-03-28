#pragma once 

#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

namespace ns_sock
{
    class Sock
    {
        public:
            static int Socket()
            {
                int sock = socket(AF_INET, SOCK_STREAM, 0);
                if (sock < 0)
                {
                    std::cerr << "socket error" << std::endl;
                    exit(1);
                }
                int opt = 1;
                setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
                return sock;
            }

            static bool Bind(const int& sock, const short& port)
            {
                struct sockaddr_in local; 
                memset(&local, 0, sizeof(local));
                local.sin_family = AF_INET;
                local.sin_port = htons(port);
                local.sin_addr.s_addr = INADDR_ANY;
                socklen_t len = sizeof(local);
                if (bind(sock, reinterpret_cast<struct sockaddr*>(&local), len) < 0)
                {
                    std::cerr << "bind error" << std::endl;
                    exit(2);
                }

                return true;
            }

            static bool Listen(const int& sock, const int backlog)
            {
                if (listen(sock, backlog) < 0)
                {
                    std::cerr << "listen error" << std::endl;
                    exit(3);
                }

                return true;
            }
    };
}
