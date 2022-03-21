#include <iostream>
#include <cstdlib>
#include <string>
#include <unistd.h>
using namespace std;

bool GetQueryString(std::string& query_string)
{
    bool result = false;
    std::string method = getenv("METHOD");
    if(method == "GET")
    {
        query_string = getenv("QUERY_STRING");
        result = true;
    }
    else if(method == "POST")
    {
        int content_length = atoi(getenv("CONTENT_LENGTH"));
        char ch = '\0';
        while(content_length)
        {
            read(0, &ch, 1);
            query_string += ch;
            --content_length;
        }
        result = true;
    }
    else 
    {
        result = false;
    }
    return result;
}

void CutString(std::string& in, const std::string& sep, std::string& out1, std::string& out2)
{
    size_t pos = in.find(sep);
    if(std::string::npos != pos)
    {
        out1 = in.substr(0, pos);
        out2 = in.substr(pos + sep.size());
    }
}


int main()
{
    std::string query_string;
    GetQueryString(query_string);

    std::string str1;
    std::string str2;
    CutString(query_string, "&", str1, str2);

    std::string name1;
    std::string value1;
    CutString(str1, "=", name1, value1);

    std::string name2;
    std::string value2;
    CutString(str2, "=", name2, value2);
    
    std::cout << name1 << " : " << value1 << std::endl;
    std::cout << name2 << " : " << value2 << std::endl;
    
    std::cerr << name1 << " : " << value1 << std::endl;
    std::cerr << name2 << " : " << value2 << std::endl;
    
    int x = atoi(value1.c_str());
    int y = atoi(value2.c_str()); 

    //可能向进行某种计算(计算，搜索，登陆等)，想进行某种存储(注册)
    std::cout << "<html>";
    std::cout << "<head><meta charset=\"utf-8\"></head>";
    std::cout << "<body>";
    std::cout << "<h3> " << value1 << " + " << value2 << " = "<< x+y << "</h3>";
    std::cout << "<h3> " << value1 << " - " << value2 << " = "<< x-y << "</h3>";
    std::cout << "<h3> " << value1 << " * " << value2 << " = "<< x*y << "</h3>";
    std::cout << "<h3> " << value1 << " / " << value2 << " = "<< x/y << "</h3>";
    std::cout << "<h3> " << value1 << " % " << value2 << " = "<< x%y << "</h3>";
    std::cout << "<h3> " << value1 << " & " << value2 << " = "<< (x&y) << "</h3>";
    std::cout << "<h3> " << value1 << " | " << value2 << " = "<< (x|y) << "</h3>";
    std::cout << "</body>";
    std::cout << "</html>";
    return 0;
}
