#include"Ring.hpp"
#include<cstdlib>
#include<ctime>
#include<unistd.h>

void* Consumer(void* arg)
{
  RingQueue<int>* rq = reinterpret_cast<RingQueue<int>*>(arg);
  
  while(true)
  {
    sleep(1);
    int x = 0;
    rq->Pop(x);
    std::cout << " consume done <<< " << x << std::endl;
  }
}


void* Product(void* arg)
{
  RingQueue<int>* rq = reinterpret_cast<RingQueue<int>*>(arg);
  while(true)
  {
    sleep(2);
    int x = rand()%100+1;
    rq->Push(x);
    std::cout << " product done >>> " << x <<std::endl;
  }
}


int main()
{
  srand(static_cast<unsigned int>(time(nullptr)));
  RingQueue<int> *rq = new RingQueue<int>();
  pthread_t c, p;
  pthread_create(&c, nullptr, Consumer, rq);
  pthread_create(&p, nullptr, Product, rq);
  
  pthread_join(c, nullptr);
  pthread_join(p, nullptr);
  
  
  return 0;
}
