#include"ThreadPool.hpp"
#include"task.hpp"
#include<unistd.h>
#include<cstdlib>
#include<ctime>

int main()
{
  ThreadPool<Task> *tp = new ThreadPool<Task>;
  tp->InitThreadPool();
  srand(static_cast<unsigned int>(time(nullptr)));
  const char* buff = "+-*/";
  while(true)
  {
    int x = rand()%100;
    int y = rand()%100;
    char op = buff[rand()%4];
    Task t(x, y, op);
    tp->Push(t);
    sleep(1);
  }
  return 0;
}
