#include <iostream>
#include <cstdlib>
#include <string>
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
        
    }
    else 
    {

    }

    return 0;
}
