#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
using namespace std;

int main()
{
    std::string method = getenv("METHOD");
    std::cerr << "Debug Test: " << getenv("METHOD") <<  std::endl;
    std::string query_string;
    if(method == "GET")
    {
        query_string = getenv("QUERY_STRING");
        cerr << "Debug QUERY STRING:" << query_string << std::endl;
    }
    else if(method == "POST")
    {
        std::cerr <<"Content-Length: " << getenv("CONTENT_LENGTH") << std::endl; 
        int content_length = atoi(getenv("CONTENT_LENGTH"));
        char ch = '\0';
        while(content_length)
        {
            read(0, &ch, 1);
            query_string += ch;
            --content_length;
        }
        std::cerr << query_string << std::endl;
    }
    else 
    {

    }

    return 0;
}
