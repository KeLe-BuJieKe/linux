#include"udpClient.hpp"

int main(int argc, char*argv[])
{
  if(argc != 3)
  {
    std::cerr << "Usage: " << argv[0] << "server_ip server_port" << std::endl;
    return 1;
  }
  std::string ip = argv[1];
  int port = atoi(argv[2]);

  UdpClient* ucli = new UdpClient(ip, port);
  ucli->InitUdpClient();
  ucli->Start();
  return 0;
}
