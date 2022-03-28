#include "select_server.hpp"

#include <string>
#include <cstdlib>
static void Usage(std::string proc)
{
    std::cerr << "Usage: " << "\n\t" << proc << " port" << std::endl;
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        Usage(argv[0]);
        exit(4);
    }
    unsigned short port = atoi(argv[1]);
    ns_select::SelectServer* select_svr = new ns_select::SelectServer(port);
    select_svr->InitSelectServer();
    select_svr->Run();
    return 0;
}
