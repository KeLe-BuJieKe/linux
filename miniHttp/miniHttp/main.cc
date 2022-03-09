#include "HttpServer.hpp"
#include <memory>

void Usage(std::string proc)
{
    std::cerr << "Usage: " << proc << " port" << std::endl;
}


int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        Usage(argv[0]);
        exit(1);
    }

    std::cout << "Hello Http" << std::endl;
    int port = atoi(argv[1]);
    std::shared_ptr<HttpServer> http_server(new HttpServer(port)); 
    http_server->InitHttpServer();
    http_server->Loop();
    return 0;
}
