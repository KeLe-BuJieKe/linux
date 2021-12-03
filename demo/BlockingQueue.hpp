#pragma once
#include<iostream>
#include<pthread.h>
#include<unistd.h>
#include<queue>
#include<cstdlib>
#define Num 10

template<class T>
class BlockQueue
{
  private:
    bool IsFull() 
    {
      return blockQueue.size()==m_cap;
    }
    bool IsEmpty() 
    {
      return blockQueue.empty();
    }
  public:
    BlockQueue(const int num = Num):m_cap(num)
    {
      pthread_mutex_init(&lock,nullptr);  //初始化互斥锁
      pthread_cond_init(&full,nullptr);   //初始化条件变量
      pthread_cond_init(&empty,nullptr);

    }

    int size() 
    {
      return blockQueue.size();
    }

    void push(const T & in)
    {
      pthread_mutex_lock(&lock); 
      while( IsFull() )
      {
        //不能进行生产，需要等待，等待blockQueue可以容纳新的数据
        //在特定的条件变量下等待，在等待时条件是不满足的，在进入等待的时候，会自动释放mutex互斥锁，如果不释放会造成死锁
        //而当条件满足条件唤醒，又会自动获得mutex互斥锁
        pthread_cond_wait(&full,&lock);
      }
      
      blockQueue.push(in);
    
      pthread_mutex_unlock(&lock);
      if(blockQueue.size()>m_cap/2)
      {
        pthread_cond_signal(&empty);
      }
    }

    void Pop(T & out)
    {
      pthread_mutex_lock(&lock); 
      while( IsEmpty() )
      {
        //不能进行消费，需要等待，等待blockQueue有新的数据
        pthread_cond_wait(&empty,&lock);
      }
      out = blockQueue.front();
      blockQueue.pop();
      pthread_mutex_unlock(&lock);

      if(blockQueue.size() < m_cap/2)
      {
        pthread_cond_signal(&full);
      }
    }
    ~BlockQueue()
    {
      pthread_mutex_destroy(&lock);  //销毁互斥锁
      pthread_cond_destroy(&full);   //销毁条件变量
      pthread_cond_destroy(&empty);
    }
  private:
    pthread_mutex_t lock;      //来实现互斥
    //来实现同步
    pthread_cond_t full;   
    pthread_cond_t empty;
    //缓冲区
    std::queue<T> blockQueue;
    int m_cap;
};
