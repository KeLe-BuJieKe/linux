#pragma once
#include<iostream>
#include<pthread.h>

int GetResult(int x, int y, char op)
{
  int ret = 0;
  switch(op)
  {
    case '+':
      ret = x+y;
      break;
    case '-':
      ret = x-y;
      break;
    case '*':
      ret = x*y;
      break;
    case '/':
      ret = x/y;
      break;
    default:
      break;
  }
  return ret;
}

typedef int (*handler_t) (int, int, char);
class Task
{
  private:
    int _x;
    int _y;
    char _op;
    handler_t handler = GetResult;
    pthread_mutex_t _lock = PTHREAD_MUTEX_INITIALIZER;
  public:
    Task() = default;
    
    Task(int x, int y, int op):_x(x) ,_y(y),_op(op)
    {}
    
    void LockQueue() //加锁
    {
      pthread_mutex_lock(&_lock);
    }

    void UnlockQueue() //解锁
    {
      pthread_mutex_unlock(&_lock);
    }
    void Run()
    {
      if(_op == '/' && _x == 0)
      {
        LockQueue(); 
        std::cerr << " div error " << std::endl;
        UnlockQueue();
        return;
      }
      int ret = handler(_x, _y, _op);
      LockQueue(); 
      std::cout << "thread_id [ " << pthread_self() << " ] " << _x << _op << _y << " = " << ret << std::endl;
      UnlockQueue();
    }

    ~Task()
    {
      pthread_mutex_destroy(&_lock);
    }

};
