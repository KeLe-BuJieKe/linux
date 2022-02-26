#pragma once
#include <iostream>
#include <string>
#include <unistd.h>

class Handler
{
    public:
      Handler(){}
      ~Handler(){}
      void operator()(int& sock, int port, std::string ip)
      {
           char buffer[1024];
           while(true)
           {
               ssize_t size = read(sock, buffer, sizeof(buffer)-1);
               if(size > 0)
               {
                   buffer[size] = 0;
                   std::cout << ip << ":" << port  << "# " << buffer << std::endl;
                   
                   write(sock, buffer, size);
               }
               else if(size == 0)
               {
                   std::cout << ip << ":" << port <<" close!!!" << std::endl;
                   break;
               }
               else
               {
                   std::cerr << sock  << " read error " << std::endl;
                   break;
               }
           }
           close(sock);
      }
};

class Task
{
    private:
        int sock;
        std::string ip;
        int port;
        Handler handler;
    public:
        Task(){}
        Task(int _sock, std::string _ip, int _port)
        :sock(_sock), ip(_ip), port(_port)
        {}
        void Run()
        {
            handler(sock, port, ip);
        }
        ~Task()
        {}
        

};
