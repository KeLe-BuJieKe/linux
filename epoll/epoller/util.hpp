#pragma once 

#include <iostream>
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
}
