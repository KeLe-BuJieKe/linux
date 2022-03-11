#pragma once 

#include <iostream>
#include <unistd.h>
#include <string>
#include <vector>
#include <unordered_map>
#include <sstream>
#include "Util.hpp"
#include "Log.hpp"


#define SEP ": " 
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"

//http请求
class HttpRequest 
{
    public:
        std::string request_line; //请求行
        std::vector<std::string> request_header;//请求报头
        std::string blank; //空行
        std::string request_body; //请求正文
    
        //解析完毕之后得结果
        std::string method; //请求方法
        std::string uri; //uri  path?args
        std::string version; //http版本号
        //报头与其对应得value
        std::unordered_map<std::string, std::string> hander_key_map;
        int content_length; //正文长度
        std::string path;
        std::string query_string;
        HttpRequest():content_length(0)
        {}
        ~HttpRequest() = default;
};

//http响应
class HttpResponse
{
    public:
        std::string status_line; //状态行
        std::vector<std::string> response_header; //响应报头
        std::string blank; //空行
        std::string response_body; //响应正文
        int status_code;
};

//用于读取请求、分析请求、构建响应、IO通信
class EndPoint
{
    private:
        int sock; //套接字---文件描述符
        HttpRequest http_request; //http请求
        HttpResponse http_response;//http响应
    private:
        void RecvHttpRequestLine() //读取请求行
        {
            std::string& line = http_request.request_line;
            Util::ReadLine(sock, line);
            http_request.request_line.resize(line.size()-1);
            LOG(INFO, line);
        }

        void RecvHttpRequestHander() //读取请求报头
        {
            std::string line;
            while(true)
            {
                line.clear(); 
                Util::ReadLine(sock, line);
                if(line == "\n")
                {
                    http_request.blank = line;
                    break;
                }
                line.resize(line.size()-1); //将http自带得\n去掉
                http_request.request_header.push_back(line);
                LOG(INFO, line);
            }
        }
        
        void ParseHttpRequsetLine() //解析请求行
        { 
            HttpRequest& request = http_request;
            std::stringstream ss(request.request_line);
            ss >> request.method >> request.uri >> request.version; 
        }

        void ParseHttpRequsetHander() //解析请求报头
        {
            std::string key;
            std::string value;
            for(auto& it : http_request.request_header)
            {
                key.clear();
                value.clear();
                if(Util::CutString(it, key, value, SEP) == true)
                {
                    http_request.hander_key_map.insert(std::make_pair(key, value));
                }
            }
        }

        bool IsNeedRecvHttpRequestBody() //根据请求方法来判断是否需要读取正文部分
        {
            const std::string& method = http_request.method;
            if(method == "POST")
            {
                auto& header_kv_map = http_request.hander_key_map; 
                auto it = header_kv_map.find("Content-Length");
                if(it != header_kv_map.end())
                {
                    http_request.content_length = atoi(it->second.c_str());
                    return true;
                }
            }
            return false;
        }

        void RecvHttpRequestBody() //读取正文
        {
            if(IsNeedRecvHttpRequestBody() == true)
            {
                int content_length = http_request.content_length; 
                std::string& body = http_request.request_body;

                char ch = '\0';
                while(content_length)
                {
                    ssize_t size = recv(sock, &ch, sizeof(ch), 0);
                    if(size > 0)
                    {
                        body += ch;
                        --content_length;
                    }
                    else 
                    {
                        break;
                    }
                }
            }
        }
    public:
        EndPoint(int _sock):sock(_sock)
        {

        }

        //读取请求
        void RcvHttpRequset()
        {
            RecvHttpRequestLine(); //读取请求行
            RecvHttpRequestHander();//读取请求报头
            ParseHttpRequsetLine(); //解析请求行
            ParseHttpRequsetHander();//解析报头
            RecvHttpRequestBody();//读取正文
        }
        //生成请求
        void MakeHttpResponse()
        {
            int& code = http_response.status_code; 
            std::string temp;
            if(http_request.method != "GET" && http_request.method != "POST")
            {
                //非法请求
                LOG(WARNING, "method is no right");
                code = 404;
                goto END;
            }
            if(http_request.method == "GET") //如果是GET方法要分析此时是否带参数
            {
                size_t pos = http_request.uri.find("?");
                if(pos != std::string::npos) //带参
                {

                    Util::CutString(http_request.uri, http_request.path, http_request.query_string, "?");
                }
                else //不带参
                {
                    http_request.path = http_request.uri;
                }
            }
            temp = http_request.path;
            http_request.path = WEB_ROOT;
            http_request.path += temp;
            if(http_request.path[http_request.path.size()-1] == '/')
            {
                http_request.path += HOME_PAGE;
            }
END:
            return;
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
            LOG(INFO, "Handler Request Begin");
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
            ep->MakeHttpResponse();
            ep->SendHttpResponse();
            delete ep;
#endif 
            LOG(INFO, "Handler Request End");
            return nullptr;
        }

};
