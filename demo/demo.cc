#include"BlockingQueue.hpp"

pthread_mutex_t c_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t p_lock = PTHREAD_MUTEX_INITIALIZER;

void * Consumer (void * arg)
{
  BlockQueue<int> * bq = (BlockQueue<int> *) arg;
  while(true)
  {
    sleep(2);
    pthread_mutex_lock(&c_lock);
    int data = 0;
    bq->Pop(data);
    pthread_mutex_unlock(&c_lock);
    std::cout<<" consumer: "<<pthread_self()<<"  消费的数据为： " << data << " size: "<< bq->size() << std::endl;
  }
}

void * Producer (void * arg)
{
  BlockQueue<int> * bq = (BlockQueue<int> *) arg;
  while(true)
  {
    sleep(1);
    pthread_mutex_lock(&p_lock);
    int data = rand() % 100 + 1;
    bq->push(data);
    pthread_mutex_unlock(&p_lock);
    std::cout<< " producer: "<< pthread_self()<<" 生产的数据为：" << data << " size: " << bq->size() <<std::endl;
  }
}

int main()
{
  srand((unsigned long)time(nullptr));
  pthread_t p[3],c[2];
  BlockQueue<int>* bq = new BlockQueue<int> ();
  for(size_t i = 0; i < 3; ++i)
  {
    pthread_create(&p[i],nullptr,Producer,bq);
  }

  for(size_t j = 0; j < 2; ++j)
  {
    pthread_create(&c[j],nullptr,Consumer,bq);
  }
  

  for(size_t i = 0; i < 3; ++i)
  {
    pthread_join(p[i],nullptr);
  }

  for(size_t j = 0; j < 2; ++j)
  {
    pthread_join(c[j],nullptr);
  }
  pthread_mutex_destroy(&c_lock);
  pthread_mutex_destroy(&p_lock);

  return 0;
}
