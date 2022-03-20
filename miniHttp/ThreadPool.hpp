#pragma once 
#include <iostream>
#include <queue>
#include <pthread.h>
#include "Task.hpp"
#include "Log.hpp"

const int KNUM = 5;

class ThreadPool
{
    private:
        int num;
        bool stop;
        std::queue<Task> task_queue;
        pthread_mutex_t lock;
        pthread_cond_t cond;
        static ThreadPool* single_instance;
    private: 
        ThreadPool(int _num = KNUM):num(_num), stop(false)
        {
            pthread_mutex_init(&lock, nullptr);
            pthread_cond_init(&cond, nullptr);
        }
        
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;
    public:
        
        static ThreadPool* GetInstance()
        {
            pthread_mutex_t mymutex = PTHREAD_MUTEX_INITIALIZER;
            if (single_instance == nullptr)
            {
                pthread_mutex_lock(&mymutex);
                if (single_instance == nullptr)
                {
                    single_instance = new ThreadPool;
                    single_instance->InitThreadPool();
                }
                pthread_mutex_unlock(&mymutex);
            }
            return single_instance;
        }

        bool IsStop() const 
        {
            return stop;
        }

        void ThreadWait()
        {
            pthread_cond_wait(&cond, &lock);
        }

        void ThreadWake()
        {
           pthread_cond_signal(&cond);
        }
        
        void Lock()
        {
            pthread_mutex_lock(&lock);
        }

        void UnLock()
        {
            pthread_mutex_unlock(&lock);
        }
        
        bool IsEmpty() const 
        {
            return task_queue.empty();
        }

        static void* ThreadRoutine(void* arg)
        {
            pthread_detach(pthread_self());
            ThreadPool* tp = reinterpret_cast<ThreadPool*>(arg);

            while (true)
            {
                Task t;
                tp->Lock();
                while (tp->IsEmpty())
                {
                    tp->ThreadWait();
                }
                tp->PopTask(t);
                tp->UnLock();
                t.ProcessOn();
            }
            return nullptr;
        }

        bool InitThreadPool()
        {
            for (int i = 0; i < num; ++i)
            {
                pthread_t tid; 
                if (pthread_create(&tid, nullptr, ThreadRoutine, this) != 0)
                {
                    LOG(FATAL, "Thread Pool create error");
                    return false;
                }
            }
            LOG(INFO, "Thread Pool create success");
            return true;
        }

        void PushTask(const Task& task)
        {
           Lock(); 
           task_queue.push(task);
           UnLock();
           ThreadWake();
        }
        
        void PopTask(Task& task)
        {
            task = task_queue.front();
            task_queue.pop();
        }

        ~ThreadPool()
        {
            pthread_mutex_destroy(&lock);
            pthread_cond_destroy(&cond);
        }
};

ThreadPool* ThreadPool::single_instance = nullptr;
