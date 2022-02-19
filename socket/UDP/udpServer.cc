#include"udpServer.hpp"

int main(int argc, char*argv[])
{
  //std::string ip = "127.0.0.1"; //127.0.0.1 == localhost ：标识本主机：本地环回
  //std::string ip = "106.55.160.195"; 
  int port = atoi(argv[1]);
  UdpServer *svr = new UdpServer(port);
  svr->InitUdpServer();
  svr->Start();
  return 0;
}
