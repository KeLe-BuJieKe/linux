#pragma once 

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <fcntl.h>
#include <unistd.h>
#include "Util.hpp"
#include "Log.hpp"


#define SEP ": " 
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#define HTTP_LINE_END "\r\n"


static std::string CodeToDesc(const int code)
{
    std::string desc;
    switch(code)
    {
        case 200:
            desc = "OK";
            break;
        case 404:
            desc = "Not Found";
            break;
        default:
            break;
    }
    return desc;
}

static std::string SuffixToDesc(const std::string& suffix)
{
    static std::unordered_map<std::string, std::string> suffixtodesc 
    {
        {".html", "text/html"},
        {".css", "text/css"},
        {".js", "application/javascript"},
        {".jpg", "application/x-jpg"},
        {".xml", "application/xml"}
    };
    
    auto iter = suffixtodesc.find(suffix);
    if(iter != suffixtodesc.end())
    {
        return iter->second;        
    }
    return "text/html";
}


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
        bool cgi;
        std::string path;
        std::string query_string;
        std::string suffix; //后缀
    public:
        HttpRequest(): content_length(0), cgi(false)
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
        int file_fd;
        int size;
        HttpResponse():blank(HTTP_LINE_END), status_code(200), file_fd(-1), size(0)
        {}
        ~HttpResponse()
        {}
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
            transform(request.method.begin(), request.method.end(), request.method.begin(), ::toupper);
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

        int ProcessNonCgi(const int size)
        {
            http_response.file_fd = open(http_request.path.c_str(), O_RDONLY);
            if(http_response.file_fd > 0)
            {
                http_response.status_line = HTTP_VERSION;
                http_response.status_line += " ";
                http_response.status_line += std::to_string(http_response.status_code);
                http_response.status_line += " ";
                http_response.status_line += CodeToDesc(http_response.status_code);
                http_response.status_line += HTTP_LINE_END;
                http_response.size = size;
                
                std::string hander_line = "Content-Length: ";
                hander_line += std::to_string(size);
                hander_line += HTTP_LINE_END;
                http_response.response_header.push_back(hander_line);
                
                hander_line = "Content-Type: ";
                hander_line += SuffixToDesc(http_request.suffix);
                hander_line += HTTP_LINE_END;
                http_response.response_header.push_back(hander_line);
                return 200;
            }
            return 404;  
        }
        int ProcessCgi(const int size)
        {

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
            int& code = http_response.status_code;//状态码 
            std::string temp;
            struct stat st;
            int size;
            size_t found;
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
                    http_request.cgi = true;
                    Util::CutString(http_request.uri, http_request.path, http_request.query_string, "?");
                }
                else //不带参
                {
                    http_request.path = http_request.uri;
                }
            }
            else if(http_request.method == "POST")
            {
                http_request.cgi = true;
            }
            else 
            {
                //nothing to do
            }

            temp = http_request.path;
            http_request.path = WEB_ROOT; //添加我们指定得根目录
            http_request.path += temp;
            if(http_request.path[http_request.path.size()-1] == '/') //如果要访问根目录那么给它显示起始页面
            {
                http_request.path += HOME_PAGE;
            }

            if(stat(http_request.path.c_str(), &st) == 0) //说明资源是存在的
            {
                if(S_ISDIR(st.st_mode))
                {
                    //说明请求的资源是一个目录,不能允许它访问该目录下所有的文件
                    //让它访问该木下的起始页面
                    //虽然是一个文件，但绝对不会以/结尾，因为在上面已经处理过了
                    http_request.path += '/';
                    http_request.path += HOME_PAGE;
                    stat(http_request.path.c_str(), &st);
                }
                //判断它是否是可执行文件，就需要特殊处理
                if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
                {
                    http_request.cgi = true;
                }
                size = st.st_size;
            }
            else 
            {
                //说明资源不存在
                std::string temp = http_request.path;
                temp += " Not Found";
                LOG(WARNING, temp);
                code = 404;
                goto END;
            }
           
            //获取文件后缀
            found = http_request.path.rfind(".");
            if(found == std::string::npos)
            {
                http_request.suffix = ".html";
            }
            else 
            {
                http_request.suffix = http_request.path.substr(found);
            }
            if(http_request.cgi == true) //判断是否需要cgi
            {
                code = ProcessCgi(size);
            }
            else 
            {
                //1.目标网页一定存在的。返回静态网页
                //2.返回并不是简单的返回，而是去构建Http响应返回
                code = ProcessNonCgi(size);
            }
END:
            if(code != 200)
            {
                
            }
            return;
        }

        //发送请求
        void SendHttpResponse()
        {
            send(sock, http_response.status_line.c_str(), http_response.status_line.size(), 0);
            for(auto& it : http_response.response_header)
            {
                send(sock, it.c_str(),  it.size(), 0);
            }
            send(sock, http_response.blank.c_str(), http_response.blank.size(), 0);
            sendfile(sock, http_response.file_fd, nullptr, http_response.size);
            close(http_response.file_fd);
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
