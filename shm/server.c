#include"comm.h"
int main()
{
  //1、首先手动生成唯一一个标识
  //这个参数的路径必须要存在，存在的路径可以随便设置
  //PROJ_ID也可以随便设置
  key_t key=ftok(PATH_NAME,PROJ_ID);
  if(key<0)
  {
    perror("ftok error\n");
    return 1;
  }
  printf("%x",key);
  //2、生成共享内存，权限为666，
  //选项组合含义为：当这个key对应的共享内存存在时，则出错，否则创建一个新的共享内存
  //这样就能够保证我们创建的共享内存一定是最新生成的
  int shmid=shmget(key,SIZE,IPC_CREAT|IPC_EXCL|0666);
  if(shmid<0)
  {
      perror("shmget error\n");
      return 2;
  }

  //3、利用shmat函数来关联到刚刚创建的共享内存，返回的是共享内存的起始地址
  //第二个参数代表的是：你自己并不知道你要映射到那个区，所有设置为NULL，让系统默认映射
  //第三个参数是：关联这个共享内存给这个共享内存设置某些属性，填入0表示使用默认
  char *s =(char*) shmat(shmid,NULL,0);
  
  while(1)
  {
    printf("%s\n",s);
    sleep(1);
  }
  //4、利用shmdt函数来去关联到刚刚创建的共享内存，成功返回0，失败返回-1
  shmdt(s);
  sleep(10);

  //5、利用shmctl函数来删除创建的共享内存，如果不去主动删除，该共享内存会一直存在，直到服务器内核被重启
  shmctl(shmid,IPC_RMID,NULL);
  sleep(10);
  return 0;
}
