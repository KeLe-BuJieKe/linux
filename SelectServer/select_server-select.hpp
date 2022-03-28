#pragma once 
#include <sys/select.h>
#include "sock.hpp"
#include <unordered_map>

const int BACK_LOG = 5;
const int NUM = sizeof(fd_set) * 8;
const int DEL_FD = -1;



namespace ns_select
{

    class SelectServer
    {
        private:
            int listen_sock;
            int port;
        public:
            SelectServer(unsigned short _port):port(_port)
            {
            }

            void InitSelectServer()
            {
                listen_sock = ns_sock::Sock::Socket();
                ns_sock::Sock::Bind(listen_sock, port);
                ns_sock::Sock::Listen(listen_sock, BACK_LOG);
            }
            
            void ClearArray(int fd_array[], int num, int default_fd)
            {
                for (int i = 0; i < num; ++i)
                {
                    fd_array[i] = default_fd;
                }
            }

            void Run()
            {
                fd_set rfds; 
                int fd_array[NUM] = {0};
                //用来初始化数据中的所有fd
                ClearArray(fd_array, NUM, DEL_FD);
                //把监听套接字sock写入数组的第一个元素
                fd_array[0] = listen_sock;

                for ( ; ; )
                {
                    //时间也是输出输出参数，所以如果要是间隔性的timeout返回，那么就需要对时间也进行重新设定
                    struct timeval timeout = {5, 0}; //每隔5秒timeout一次
                    int max_fd = DEL_FD;
                    FD_ZERO(&rfds); //情况所有的read_fd
                    //第一次循环的时候，我们fd_array数组中至少已经有了已经有了一个fd,listen_sock
                    for (int i = 0; i < NUM; ++i)
                    {
                        if (fd_array[i] == DEL_FD)
                        {
                            continue; 
                        }
                        else 
                        {   
                            //找出最大的文件描述符
                            max_fd = fd_array[i] > max_fd ? fd_array[i] : max_fd;
                            //将合法的fd添加进去
                            FD_SET(fd_array[i], &rfds); 
                        }
                    }
                    //1.select 阻塞等待 timeval参数填nullptr
                    //2.timeval = {0} 非阻塞轮询
                    //3.阻塞+轮询，timeval={5，0}，阻塞等待。5s之后，select返回，无论是否有事件发生
                    switch (select(max_fd + 1, &rfds, nullptr, nullptr, nullptr/*&timeout*/))
                    {
                        case 0:
                            std::cout << "timeout" << std::endl;
                            break;
                        case -1:
                            std::cerr << "select error" << std::endl;
                            break;
                        default:
                            //正常的情况处理
                            //std::cout << "有事情发生... timeout: " << timeout.tv_sec << std::endl;    
                            HandlerEven(rfds, fd_array, NUM);
                            break;
                    } // end switch 
                } // end for 
            } // end functional 

            ~SelectServer()
            {
                if (listen_sock >= 0)
                {
                    close(listen_sock);
                }
            }

            void HandlerEven(const fd_set& rfds, int fd_array[], const int& num)
            {
                for (int i = 0; i < num; ++i)
                {
                    //过滤不需要的fd
                    if (fd_array[i] == DEL_FD)
                    {
                        continue;
                    }
                    //是一个合法的fd,但是不一定就绪了
                    if (fd_array[i] == listen_sock && FD_ISSET(fd_array[i], &rfds))
                    {
                        //是一个合法的fd,并且已经就绪了，是链接事件到来了
                        //我们需要进行accept
                        struct sockaddr_in peer;
                        socklen_t len = sizeof(peer);
                        int sock = accept(listen_sock, reinterpret_cast<struct sockaddr*>(&peer), &len);
                        if (sock < 0)
                        {
                            std::cerr << "accept error "<< std::endl;
                            continue;
                        }
                        unsigned short peer_port = ntohs(peer.sin_port);
                        std::string peer_ip = inet_ntoa(peer.sin_addr);
                        std::cout << "get a new link "<< peer_ip << ":" << peer_port << std::endl;
          
                        //不可以进行对应的recv,recv是IO，等+拷贝,select知道数据有没有就绪
                        //而是将该文件描述符添加到fd_array数组当中
                        if (AddFdToArray(fd_array, num, sock) == true)
                        {
                            
                        }
                        else 
                        {
                            close(sock);
                            std::cerr << "select server is full, close fd: " << sock << std::endl;
                        }
                    }
                    else if(FD_ISSET(fd_array[i], &rfds)) //是一个合法的fd,并且已经就绪了，是读取事件就绪
                    {
                        //处理正常的fd,并且数据已经就绪了，是读数据事件就绪
                        //实现读，并且一定不会被阻塞
                        char buffer[1024];
                        //这里不能确定读完了发送过来的请求
                        //如果一条链接发给了多个请求数据，但是每隔都只有10个字节，这样就会出现粘包问题
                        //如果没有读到一个完整的报文，数据可能会丢失
                        //这里我们怎么保证自己能拿到完整的数据呢？
                        //1、定制协议
                        //2、还要给每一个sock定义一个缓冲区
                        ssize_t s = recv(fd_array[i], buffer, sizeof(buffer)-1, 0);
                        if (s > 0)
                        {
                            buffer[s] = '\0';
                            std::cout << "echo# " << buffer << std::endl;
                        }
                        else if(s == 0)
                        {
                            std::cout << "client quit" << std::endl;
                            close(fd_array[i]);
                            fd_array[i] = DEL_FD; //清除数组中的文件描述符
                        }
                        else 
                        {
                            std::cerr << "recv error " << std::endl;
                            close(fd_array[i]);
                            fd_array[i] = DEL_FD; //清除数组中的文件描述符
                        }
                    }
                    else 
                    {
                        //TO DO
                    }
                } //end for
            }

            bool AddFdToArray(int fd_array[], int num, int sock)
            {
                for (int i = 0; i < num; ++i)
                {
                    if (fd_array[i] == DEL_FD)
                    {
                        fd_array[i] = sock;
                        return true;
                    }
                }
                return false;
            }
    };
}
