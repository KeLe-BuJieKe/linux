#pragma once 
#include "sock.hpp"
#include <sys/epoll.h>
#include <cstring>
namespace ns_epoll
{
    const int MAX_NUM = 64;
    const int BACK_LOG = 5;
    class EpollServer
    {
        private:
            int listen_sock;
            int epfd;
            uint16_t port;
        public:
            EpollServer(int _port):listen_sock(-1), epfd(-1), port(_port)
            {}
            
            ~EpollServer()
            {
                if (listen_sock >= 0)
                {
                    close(listen_sock);
                }

                if (epfd >= 0)
                {
                    close(epfd);
                }
            }//destructor end
        public:
            void InitEpollServer()
            {
                listen_sock = ns_sock::Sock::Socket();
                port = ns_sock::Sock::Bind(listen_sock, port);
                ns_sock::Sock::Listen(listen_sock, BACK_LOG);
                
                if ((epfd = epoll_create(256)) < 0)
                {
                    std::cerr << "create epoll" << std::endl;
                    exit(4); 
                }
            }//InitEpollServer End
          
            void AddEvent(int fd, uint32_t event)
            {
                struct epoll_event ev;
                ev.events = 0;
                ev.events |= event;
                ev.data.fd = fd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
                {
                    std::cerr << "epoll_ctl error , fd: " << fd << std::endl;
                }
            }
            

            void DelEvent(int sock)
            {
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr) < 0)
                {
                    std::cerr << "epoll_ctl error fd: " << sock << std::endl;
                }
            }

            void Run()
            {
                //在这里我们目前只有一个socket是能够关心读写的，listen_sock, read event
                AddEvent(listen_sock, EPOLL_CTL_ADD); 
                int timeout = 1000;
                struct epoll_event revs[MAX_NUM];
                for ( ; ; )
                {
                    //返回值num表明有多少个事件就绪了，内核会按顺序将就绪事件依次放入revs当中
                    int num = epoll_wait(epfd, revs, MAX_NUM, timeout);
                    if (num > 0)
                    {
                        std::cout << "有事件发生了..." << std::endl;
                        for (int i = 0; i < num; ++i)
                        {
                            int sock = revs[i].data.fd;
                            if (revs[i].events & EPOLLIN) //读事件就绪
                            {
                                if (sock == listen_sock)
                                {
                                    //1.listen_sock 链接事件就绪
                                    struct sockaddr_in peer;
                                    memset(&peer, 0, sizeof(peer));
                                    socklen_t len = sizeof(peer);
                                    int fd = accept(sock, reinterpret_cast<struct sockaddr*>(&peer), &len);
                                    if (fd < 0)
                                    {
                                        std::cerr << "accept error" << std::endl;
                                        continue;
                                    }
                                    std::cout << "get a new link: " << inet_ntoa(peer.sin_addr) << ":" << ntohs(peer.sin_port) << std::endl; 
                                    AddEvent(fd, EPOLLIN);//先进行读取，只有需要写入时，才主动设置EPOLLOUT
                                }
                                else 
                                {
                                    //2.sock 普通可读事件就绪
                                    char buffer[1024];
                                    ssize_t s = recv(sock, buffer, sizeof(buffer)-1, 0);
                                    if (s > 0)
                                    {
                                        buffer[s] = '\0';
                                        std::cout << buffer << std::endl;
                                    }
                                    else if (s == 0) 
                                    {
                                        std::cout << "recv end" << std::endl;
                                        close(sock);
                                        DelEvent(sock);
                                    }
                                    else 
                                    {
                                        std::cerr << "client close" << std::endl;
                                        close(sock);
                                        DelEvent(sock);
                                    }
                                }
                            }
                            else if (revs[i].events & EPOLLOUT)
                            {
                                
                            }
                            else 
                            {
                                
                            }// if end 
                        }//for end 
                    }
                    else if (num == 0)
                    {
                        std::cout << "timeout" << std::endl;
                    }
                    else 
                    {
                        std::cerr << "epoll error" << std::endl;
                    }//if end
                }//for end
            }//Run end
    }; //EpollServer endl 
}
