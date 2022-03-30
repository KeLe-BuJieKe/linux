#include "epoll_server.hpp"
#include <string>
#include <cstdlib>

static void Usage(std::string proc)
{
    std::cout << "Usage: " << "\n\t" << proc << " port" << std::endl; 
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(5);
    }

    int port = atoi(argv[1]);
    ns_epoll::EpollServer* ep_svr = new ns_epoll::EpollServer(port);
    ep_svr->InitEpollServer();
    ep_svr->Run();
    return 0;
}
