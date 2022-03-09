#pragma once 

#include <iostream>
#include <unistd.h>
#include "Util.hpp"
#include <string>
#include <vector>

class HttpRequest 
{
    public:
        std::string request_line; //请求行
        std::vector<std::string> request_header;//请求报头
        std::string blank; //空行
        std::string request_body; //请求正文
};

class HttpResponse
{
    public:
        std::string status_line; //状态行
        std::vector<std::string> response_header; //响应报头
        std::string blank; //空行
        std::string response_body; //响应正文

};

//读取请求、分析请求、构建响应、IO通信
class EndPoint
{
    private:
        int sock;
        HttpRequest http_request;
        HttpResponse http_response;
    private:
        void RecvHttpRequestLine()
        {
            Util::ReadLine(sock, http_request.request_line);
        }

        void RecvHttpRequestHander()
        {
            
        }
    public:
        EndPoint(int _sock):sock(_sock)
        {

        }

        //读取请求
        void RcvHttpRequset()
        {

        }
        //分析请求
        void ParseHttpRequset()
        {

        }
        //生成请求
        void MakeHttpResponse()
        {

        }
        //发送请求
        void SendHttpResponse()
        {

        }
        ~EndPoint()
        {
            close(sock);
        }
};


class Entrance
{
    public:
        static void* HandlerRequest(void* arg)
        {
            pthread_detach(pthread_self());
            int sock = *(reinterpret_cast<int*>(arg));
            delete reinterpret_cast<int*>(arg);

#ifdef DEBUG 
            char buffer[1024*4];
            recv(sock, buffer, sizeof(buffer), 0);
            std::cout << "----------------------begin------------------------" << std::endl;
            std::cout << buffer << std::endl; 
            std::cout << "----------------------e n d------------------------" << std::endl;
#else
            EndPoint* ep = new EndPoint(sock);
            ep->RcvHttpRequset();
            ep->ParseHttpRequset();
            ep->MakeHttpResponse();
            ep->SendHttpResponse();
            delete ep;
#endif
            return nullptr;
        }

};
