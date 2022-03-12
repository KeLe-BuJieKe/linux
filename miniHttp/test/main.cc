#include <iostream>
#include <string>
#include <sstream>
#include <algorithm>

void test_sstream()
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

}

void test_transform()
{
    std::string method = "Get"; 
    std::cout << "method :" << method << std::endl;
    transform(method.begin(), method.end(), method.begin(), ::toupper);

    std::cout << "method :" << method << std::endl;
}

int main()
{
    //test_sstream();
    //test_transform();
    std::cout << sizeof(int*) << std::endl;
    return 0;
}
