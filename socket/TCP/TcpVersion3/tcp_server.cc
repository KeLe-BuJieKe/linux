#include"tcp_server.hpp"

int main(int argc, char* argv[])
{
    if(argc != 2)
    {
        std::cerr << "Usage :" << argv[0] << " port" << std::endl; 
        exit(1);
    }
    int port = atoi(argv[1]);

    TcpServer* tcp = new TcpServer(port);
    tcp->InitTcpServer();
    tcp->Loop();
    delete tcp;

    return 0;
}
