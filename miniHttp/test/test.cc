#include<iostream>
#include <unistd.h>

int main()
{
    execl("./a.out", NULL);
    return 0;
}
