#include"comm.h"

int main()
{
  key_t key=ftok(PATH_NAME,PROJ_ID);
  if(key<0)
  {
    perror("ftok error\n");
    return 1;
  }

  int shmid=shmget(key,SIZE,IPC_CREAT);

  if(shmid<0)
  {
    perror("shmget error\n");
    return 2;
  }
  
  char*str=(char*)shmat(shmid,NULL,0);
  
  char c='a';
  for(;c<='z';++c)
  {
    str[c-'a']=c;
    if(c=='c')
    {
      ++c;
      str[c-'a']='\0';
    }
    sleep(3);
  }
  shmdt(str);
  return 0;
}
