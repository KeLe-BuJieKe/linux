#include"tcp_client.hpp"


int main(int argc, char* argv[])
{
    if(argc != 3)
    {
        std::cerr << "Usage:" << argv[0] << " server_ip  server_port" << std::endl; 
        exit(1);
    }
    
    TcpClient tcli(argv[1], atoi(argv[2]));
    tcli.InitTcpClient();
    tcli.Start();

    return 0;
}
