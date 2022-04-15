#pragma once 

#include <iostream>
#include <vector>
#include <unistd.h>
#include <fcntl.h>

namespace ns_util
{
    void SetNotBlock(int sock)
    {
        int fl = fcntl(sock, F_GETFL);
        if (fl < 0)
        {
            std::cerr << "fcntl error " << std::endl;
            return;
        }
        else 
        {
            fcntl(sock, F_SETFL, fl | O_NONBLOCK);
        }
    }

    class StringUtil{
        
        public:
            static void Spilt(std::string& in, std::vector<std::string>& out, std::string sep)
            {
                size_t pos = -1;
                while (true){
                    pos = in.find(sep);
                    if (pos == std::string::npos)
                    {
                        break;
                    }
                    std::string temp = in.substr(0, pos);
                    in.erase(0, pos + sep.size());
                    out.push_back(temp);
                }
            }
            
            static void Deserialize(std::string &in, int *x, int *y){
                size_t pos = in.find("+");
                std::string one = in.substr(0, pos);
                std::string two = in.substr(pos + 1);

                *x = atoi(one.c_str());
                *y = atoi(two.c_str());
            }
    };
}
