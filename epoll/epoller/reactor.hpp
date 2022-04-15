#pragma once 
#include "sock.hpp"
#include <sys/epoll.h>
#include <cstring>
#include <string>
#include <unordered_map>

namespace ns_epoll
{
    class Epoller; 
    class EventItem;
    //回调函数指针类型
    typedef int (*callback_t) (EventItem*);

    class EventItem
    {
        public:
            //与通信相关
            int sock;
            //回指Epoller
            Epoller* R;

            //有关数据处理的回调函数，用来进行逻辑解耦！
            //应用数据就绪等通信细节，和数据的处理模块使用该方法进行解耦
            callback_t recv_handler;
            callback_t send_handler;
            callback_t error_handler;

            std::string inbuffer; //读取到的数据，缓冲区
            std::string outbuffer;//待发送的数据缓冲区

        public:
              EventItem():sock(-1), R(nullptr), recv_handler(nullptr)
                          ,send_handler(nullptr), error_handler(nullptr)
              {}

              ~EventItem()
              {
              }

              void MangerCallBack(callback_t _recv, callback_t _send, callback_t _error)
              {
                  this->recv_handler = _recv;
                  this->send_handler = _send;
                  this->error_handler = _error;
              }
    };

    const int MAX_NUM = 64;
    class Epoller
    {
        private:
            int epfd;
            //sock 与EventItem的映射 
            std::unordered_map<int, EventItem> event_items;
        public:
            Epoller():epfd(-1)
            {}
            
            ~Epoller()
            {
                if (epfd >= 0)
                {
                    close(epfd);
                }
            }//destructor end

        public:
            void InitEpoller()
            {
                if ((epfd = epoll_create(256)) < 0)
                {
                    std::cerr << "create epoll" << std::endl;
                    exit(4); 
                }
            }//InitEpoller End
          
            void AddEvent(int fd, uint32_t event, const EventItem& item)
            {
                struct epoll_event ev;
                ev.events = 0;
                ev.events |= event;
                ev.data.fd = fd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev) < 0)
                {
                    std::cerr << "epoll_ctl error , fd: " << fd << std::endl;
                }
                else 
                {
                    event_items.insert({fd, item});
                }
            }
            

            void DelEvent(int sock)
            {
                if (epoll_ctl(epfd, EPOLL_CTL_DEL, sock, nullptr) < 0)
                {
                    std::cerr << "epoll_ctl error fd: " << sock << std::endl;
                }

                event_items.erase(sock);
            }
            
            void EnableReadWrite(int sock, bool read, bool write)
            {
                struct epoll_event ev;
                ev.events = (read ? EPOLLIN : 0) | (write ? EPOLLOUT : 0) | EPOLLET;
                ev.data.fd = sock;
                if (epoll_ctl(epfd, EPOLL_CTL_MOD, sock, &ev) < 0){
                    std::cerr << "epoll_ctl mod error, fd:" << sock << std::endl; 
                }

            }

            void Dispatcher(int timeout)
            {
                //在这里我们目前只有一个socket是能够关心读写的，listen_sock, read event
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
                            if ((revs[i].events & EPOLLERR) || (revs[i].events & EPOLLHUP)){
                              if (event_items[sock].error_handler != nullptr){
                                  event_items[sock].error_handler(&event_items[sock]);
                              }
                              break;
                            }

                            if (revs[i].events & EPOLLIN)
                            {
                              if (event_items[sock].recv_handler != nullptr){
                                event_items[sock].recv_handler(&event_items[sock]);    
                              }
                            }

                            if (revs[i].events & EPOLLOUT)
                            {
                              if (event_items[sock].send_handler != nullptr)
                              {
                                event_items[sock].send_handler(&event_items[sock]);
                              }
                            }
                        }//for end 
                    }
                }//for end
            }//Run end
    }; //Epoller endl 
}
