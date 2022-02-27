#include <iostream>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include <fstream>
#include <string>
using namespace std;

int main()
{
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    if(listen_sock < 0)
    {
        cout << "socket error " << endl;
        return 1;
    }
    
    struct sockaddr_in local;
    memset(&local, 0, sizeof(local));
    local.sin_family = AF_INET;
    local.sin_port = htons(8081);
    local.sin_addr.s_addr= INADDR_ANY;

    if(bind(listen_sock, reinterpret_cast<struct sockaddr*>(&local), sizeof(local)) < 0)
    {
        std::cout << "bind error" << std::endl;
        return 2;
    }

    if(listen(listen_sock, 5) < 0)
    {
        std::cout << "listen error " << std::endl;
        return 3;
    }


    struct sockaddr_in peer;
    socklen_t len;
    for( ; ; )
    {
        int sock = accept(listen_sock, reinterpret_cast<struct sockaddr*>(&peer), &len);
        {
            if(sock < 0)
            {
                continue;
                std::cout << "accept error" << std::endl;
            }
            
            if(fork() == 0)
            {
                close(listen_sock);
                if(fork() > 0)
                {
                    exit(0);
                }
                char buffer[1024];
                recv(sock, buffer, sizeof(buffer), 0);
                std::cout << "######################################## http server begin ###########################################" << std::endl; 
                std::cout << buffer << std::endl;
                std::cout << "######################################## http server end   ###########################################" << std::endl; 
                
                std::ifstream in("index.html");
                if(in.is_open())
                {
                    in.seekg(0, std::ios::end);
                    size_t len = in.tellg(); 
                    in.seekg(0, std::ios::beg);
                    char* file  = new char[len];
                    
                    in.read(file, len);
                    in.close();
                    
                    std::string status_line = "http/1.1 307 Temporary Redirect\n";
                    std::string response_header = "Content_Length: " + std::to_string(len);
                    response_header += "\n";
                    response_header += "location: https://www.qq.com\n";
                    std::string blank = "\n";

                    send(sock, status_line.c_str(), status_line.size(), 0);
                    send(sock, response_header.c_str(), response_header.size(), 0);
                    send(sock, blank.c_str(), blank.size(), 0);
                    
                    send(sock, file, len, 0);
                    delete[] file;
                }
                close(sock);
                exit(0);
            }
            close(sock);
            waitpid(-1, nullptr, 0);
        }
    }
    return 0;
}
