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
                    if(ch == '\r') // /r/n
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
                    return 0;
                }
                else
                {
                    return -1;
                }
            }
            return static_cast<int>(out.size());   
        }
};
