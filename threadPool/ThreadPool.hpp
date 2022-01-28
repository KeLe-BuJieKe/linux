#pragma once

#include<iostream>
#include<queue>
#include<pthread.h>
#define NUM 5 
template<class T>
class ThreadPool
{
  private:
    int _threadNum; //线程的个数
    std::queue<T> _taskQueue; //任务队列
    pthread_mutex_t _lock; //互斥锁 
    pthread_cond_t _cond; //条件变量
  public:
    ThreadPool(int num = NUM):_threadNum(num)
    {
      pthread_mutex_init(&_lock, nullptr);
      pthread_cond_init(&_cond, nullptr);
    }
    void InitThreadPool() //初始化线程池
    {
      pthread_t tid;
      for(size_t i = 0; i < _threadNum; ++i)
      {
        pthread_create(&tid, nullptr, Routine, this); //创建线程
      }
    }

    void LockQueue() //加锁
    {
      pthread_mutex_lock(&_lock);
    }

    void UnlockQueue() //解锁
    {
      pthread_mutex_unlock(&_lock);
    }
    
    bool IsEmpty() //判断任务队列是否为空
    {
      return _taskQueue.empty();
    }

    void Wait() //等待 
    {
      pthread_cond_wait(&_cond, &_lock);
    }
    
    void WaitUp() //唤醒一个在等待队列当中的线程
    {
      pthread_cond_signal(&_cond);
    }

    static void* Routine(void* arg)
    {
      pthread_detach(pthread_self()); //将线程分离
      ThreadPool* self = reinterpret_cast<ThreadPool* >(arg);
      while(true)
      {
        self->LockQueue();
        while(self->IsEmpty()) //如果为空，则让线程去等待
        {
          self->Wait();
        }
        T tmp;
        self->Pop(tmp);   //从任务队列中拿到任务
        self->UnlockQueue();
        
        //处理任务
        tmp.Run();  
      }
    }

    void Push(const T& in) //用于外部将任务放入到任务队列当中
    {
      LockQueue();
      _taskQueue.push(in);
      UnlockQueue();
      WaitUp(); //唤醒一个线程
    }

    void Pop(T& out) //用来给外部获取任务，并将获取到的任务弹出
    {
      out = _taskQueue.front();
      _taskQueue.pop();
    }

    ~ThreadPool()
    {
      pthread_mutex_destroy(&_lock);
      pthread_cond_destroy(&_cond);
    }
};
