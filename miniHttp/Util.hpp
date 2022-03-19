#pragma once 

#include <iostream>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>
//工具类
class Util
{
    public:
        //按行读取文件的当中的内容
        static int ReadLine(int sock, std::string& out)
        {
            char ch = '\0';
            while(ch != '\n')
            {
                ssize_t size = recv(sock, &ch, 1, 0);
                if(size > 0)
                {
                    if(ch == '\r') 
                    {
                        recv(sock, &ch, 1, MSG_PEEK);
                        if(ch == '\n') // /r/n
                        {
                            recv(sock, &ch, 1, 0);
                        }
                        else  // /r
                        {
                            ch = '\n';
                        }
                    }
                    //普通字符
                    out += ch;
                }
                else if(size == 0)
                {
                    break; 
                }
                else
                {
                    return -1;
                }
            }
            return static_cast<int>(out.size());   
        }


        static bool CutString(const std::string& target, std::string& sub1_out, std::string& sub2_out, const std::string sep)
        {
            size_t pos = target.find(sep);
            if(pos != std::string::npos)
            {
                sub1_out = target.substr(0, pos);
                sub2_out = target.substr(pos + sep.size());
                return true;
            }

            return false;
        }
};
