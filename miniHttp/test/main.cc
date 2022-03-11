#include <iostream>
#include <string>
#include <sstream>

int main()
{
    std::string line = "Get /a/b/c/index.html http/1.1";
    std::string method;
    std::string uri;
    std::string version;

    std::stringstream ss(line);
    ss >> method >> uri >> version;
    std::cout << method << std::endl;
    std::cout << uri << std::endl;
    std::cout << version << std::endl;

    return 0;
}
