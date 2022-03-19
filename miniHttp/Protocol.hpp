#pragma once 

#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <sstream>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "Util.hpp"
#include "Log.hpp"


#define SEP ": " 
#define WEB_ROOT "wwwroot"
#define HOME_PAGE "index.html"
#define HTTP_VERSION "HTTP/1.0"
#define PAGE_404 "404.html"
#define HTTP_LINE_END "\r\n"


enum ERR_CODE
{
    OK = 200,
    BAD_REQUEST = 400,
    NOT_FOUND = 404,
    SERVER_ERROR = 500
};

//状态码与状态码描述
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

//后缀与文件类型转换
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
        int size;//要访问的文件的字节数
    public:
        HttpRequest(): content_length(0), cgi(false), size(0)
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
        int status_code; //状态码
        int file_fd; //与之响应所开辟的套接字
        HttpResponse():blank(HTTP_LINE_END), status_code(200), file_fd(-1)
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
        bool stop;
    private:
        bool RecvHttpRequestLine() //读取请求行
        {
            std::string& line = http_request.request_line;
            if(Util::ReadLine(sock, line) > 0)
            {
                //去除掉最后的\n
                http_request.request_line.resize(line.size()-1);
                LOG(INFO, line);
            }
            else 
            {
                stop = true;
            }
            return stop;
        }

        bool RecvHttpRequestHander() //读取请求报头 key:value 
        {
            std::string line;
            while(true)
            {
                line.clear(); 
                if(Util::ReadLine(sock, line) <= 0)
                {
                    stop = true;
                    break;
                }
                if(line == "\n")
                {
                    http_request.blank = line;
                    break;
                }
                line.resize(line.size()-1); //将http自带得\n去掉
                http_request.request_header.push_back(line);
                LOG(INFO, line);
            }
            return stop;
        }
        
        void ParseHttpRequsetLine() //解析请求行
        { 
            HttpRequest& request = http_request;
            //默认以空格做分割，来实现字符串分隔
            std::stringstream ss(request.request_line);
            ss >> request.method >> request.uri >> request.version; 
            //将http请求方法默认设置为全大写
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

        bool RecvHttpRequestBody() //读取正文
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
                        stop = true;
                        break;
                    }
                }
                LOG(INFO, body); 
            }
            return stop;
        }

        int ProcessNonCgi()
        {
            http_response.file_fd = open(http_request.path.c_str(), O_RDONLY);
            if(http_response.file_fd >= 0)
            {
                return OK;
            }
            return NOT_FOUND;  
        }

        int ProcessCgi()
        {
            std::string& filepath = http_request.path;//要执行的目标文件，一定存在
            std::string query_string_env;
            std::string method_env;
            std::string content_length_env;
            //站在父进程角度 
            int input[2];
            int output[2];
            int code = OK; 

            if(pipe(input) < 0)
            {
                LOG(ERROR, "pipe input error");
                code = SERVER_ERROR;
                return code;
            }
            
            if(pipe(output) < 0)
            {
                close(input[0]);
                close(input[1]);
                LOG(ERROR, "pipe output error");
                code = SERVER_ERROR;
                return code;
            }

            pid_t child = fork(); 
            if(child > 0) //father
            {
                close(input[1]);
                close(output[0]);
                
                if(http_request.method == "POST")
                {
                    const char* str = http_request.request_body.c_str();
                    
                    int content_length = http_request.request_body.size();
                    int total = 0;
                    int size = 0;
                    while(total < content_length && (size = write(output[1], str+total, http_request.request_body.size() - total)) > 0)
                    {
                        total += size;
                    }
                }
                
                char ch = '\0'; //读取cgi发过来的数据
                while(read(input[0], &ch, sizeof(ch)) > 0)
                {
                    http_response.response_body.push_back(ch);
                }

                int status = 0;
                pid_t wait_id = waitpid(child, &status, 0);    
                if(wait_id == child)
                {
                    if(WIFEXITED(status))
                    {
                        if(WEXITSTATUS(status) == 0)
                        {
                            code = OK;
                        }
                        else 
                        {
                            code = BAD_REQUEST;
                        }
                    }
                    else 
                    {
                        code = SERVER_ERROR;
                    }
                }

                close(input[0]);
                close(output[1]);
            }
            else if(child == 0) //child
            {
                //exec
                close(input[0]);
                close(output[1]);
                //input  写入到 -> 1 -> input[1]
                //output 读取到 -> 0 -> output[0]
                
                method_env = "METHOD=";
                method_env += http_request.method;
                putenv(const_cast<char*>(method_env.c_str()));
                if(http_request.method == "GET") //让子进程从环境变量里读取数据
                {
                    query_string_env = "QUERY_STRING="; 
                    query_string_env += http_request.query_string;
                    putenv(const_cast<char*>(query_string_env.c_str()));
                    LOG(INFO, "Get method, Add Query_String_Env");
                }
                else if(http_request.method == "POST")
                {
                    content_length_env = "CONTENT_LENGTH=";
                    content_length_env += std::to_string(http_request.content_length);
                    putenv(const_cast<char*>(content_length_env.c_str()));
                    LOG(INFO, "Post Method, Add Content_Lenght_Env");
                }

                dup2(input[1], 1);
                dup2(output[0], 0);
                execl(filepath.c_str(), filepath.c_str(), nullptr);
                exit(1);
            }
            else 
            {
                //fork() error
                close(input[0]);
                close(input[1]);
                close(output[0]);
                close(output[1]);
                LOG(ERROR, "fork error");
                code = SERVER_ERROR;
            }

            return code;
        }
        
        void BuildOkResponse()
        {
            std::string line = "Content-Type: ";
            line += SuffixToDesc(http_request.suffix);
            line += HTTP_LINE_END; 
            http_response.response_header.push_back(line);   
            line = "Content-Length: "; 
            if(http_request.cgi == true)
            {
                line += std::to_string(http_response.response_body.size());
            }
            else 
            {
                line += std::to_string(http_request.size); //Get
            }
            line += HTTP_LINE_END; 
            http_response.response_header.push_back(line);   
        }

        void HandlerError(std::string page)
        {
            http_request.cgi = false;
            http_response.file_fd = open(page.c_str(), O_RDONLY);
            if(http_response.file_fd >= 0)
            {
                struct stat st;
                http_request.size = st.st_size;
                stat(page.c_str(), &st);
                std::string line = "Content-Type: text/html";
                line += HTTP_LINE_END; 
                http_response.response_header.push_back(line);   
                line = "Content-Length: "; 
                line += std::to_string(st.st_size);
                line += HTTP_LINE_END; 
                http_response.response_header.push_back(line);   
            }
        }

        void MakeHttpResponseHelper()
        {
            //响应行
            std::string& status_line = http_response.status_line;
            int& code = http_response.status_code;
            status_line += HTTP_VERSION;
            status_line += " ";
            status_line += std::to_string(code);
            status_line += " ";
            status_line += CodeToDesc(code);
            status_line += HTTP_LINE_END;
            
            std::string path = WEB_ROOT;
            path += "/";
            switch(code)
            {
                case OK:
                    BuildOkResponse();
                    break;
                case BAD_REQUEST:
                    break;
                case NOT_FOUND:
                    path += PAGE_404;
                    HandlerError(path);
                    break;
                case SERVER_ERROR:
                    break;
                default:
                    break;
            }
        }

    public:
        EndPoint(int _sock):sock(_sock), stop(false)
        {}
        
        bool IsStop() const
        {
            return stop;
        }

        //读取请求
        void RcvHttpRequset()
        {
            //读取请求行和请求报头
            if(RecvHttpRequestLine() || RecvHttpRequestHander())
            {
                //recv error   nothing to do 
            }
            else 
            {
                ParseHttpRequsetLine(); //解析请求行
                ParseHttpRequsetHander();//解析报头
                RecvHttpRequestBody();//读取正文
            }
        }
        
        //生成请求
        void MakeHttpResponse()
        {
            int& code = http_response.status_code;//状态码 
            std::string temp;
            struct stat st; //用于获取请求的文件的各项属性
            size_t found;
            if(http_request.method != "GET" && http_request.method != "POST")
            {
                //非法请求
                LOG(WARNING, "method is no right");
                code = BAD_REQUEST;
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
                http_request.path = http_request.uri;
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
                http_request.size = st.st_size;
            }
            else 
            {
                //说明资源不存在
                std::string temp_path = http_request.path;
                temp_path += " Not Found";
                LOG(WARNING, temp_path);
                code = NOT_FOUND;
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
                code = ProcessCgi();
            }
            else 
            {
                //1.目标网页一定存在的。返回静态网页
                //2.返回并不是简单的返回，而是去构建Http响应返回
                code = ProcessNonCgi();
            }
END:
            MakeHttpResponseHelper();
        }
        
        //发送请求
        void SendHttpResponse()
        {
            //1.发送响应行
            //2.发送响应报头
            //3.发送空行
            //4，发送正文
            send(sock, http_response.status_line.c_str(), http_response.status_line.size(), 0);
            for(auto& it : http_response.response_header)
            {
                send(sock, it.c_str(),  it.size(), 0);
            }

            send(sock, http_response.blank.c_str(), http_response.blank.size(), 0);
            
            
            if(http_request.cgi == true)
            {
                ssize_t size = 0;
                size_t total = 0;
                std::string& response_body = http_response.response_body;
                const char* str = response_body.c_str();
                while(total < response_body.size() && (size = send(sock, str + total, response_body.size() - total, 0)) > 0)
                {
                    total += size; 
                }
            }
            else 
            {
                sendfile(sock, http_response.file_fd, nullptr, http_request.size);
            }
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
            if(ep->IsStop() == false)
            {
                LOG(INFO, "Recv No Error, Make Continue And Send");
                ep->MakeHttpResponse();
                ep->SendHttpResponse();
            }
            else 
            {
                LOG(WARNING, "Recv Error, Stop Make And Send");
            }
            delete ep;
#endif 
            LOG(INFO, "Handler Request End");
            return nullptr;
        }

};
