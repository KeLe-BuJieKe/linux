#pragma once

#include<iostream>
#include<vector>
#include<semaphore.h>
#include<pthread.h>
#define NUM 32

template<class T>
class RingQueue
{
  public:
    RingQueue(int cap = NUM):_cap(cap), _pIndex(0), _cIndex(0)
    {
      this->_q.resize(_cap);
      sem_init(&_dataSem, 0, 0);
      sem_init(&_blankSem, 0, _cap);
    }

    //生产者调用，生产数据，关系blank
    void Push(const T& in)
    {
      P(_blankSem);
      _q[_pIndex] = in;
      V(_dataSem);
      this->_pIndex++;
      this->_pIndex %= _cap;
    }
    //消费者调用，消费数据，关心data资源
    void Pop(T& out)
    {
      P(_dataSem);
      out = _q[_cIndex];
      V(_blankSem);
      this->_cIndex++;
      this->_cIndex %= _cap;
    }

    ~RingQueue()
    {
      //销毁信号量
      sem_destroy(&_dataSem);
      sem_destroy(&_blankSem);
    }
  private:
    void P(sem_t &s)
    {
      sem_wait(&s);
    }

    void V(sem_t &s)
    {
      sem_post(&s);
    }
  private:
    std::vector<T> _q; //环形队列
    int _cap; //容量
    sem_t _dataSem; //数据信号量
    sem_t _blankSem; //格子信号量
    int _cIndex;  //消费者消费位置
    int _pIndex;  //生产者生产位置
};
