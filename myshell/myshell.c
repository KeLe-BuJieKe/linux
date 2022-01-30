#include<stdio.h>
#include<string.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<ctype.h>
#include<sys/wait.h>
#include<stdlib.h>


#define SIZE 256
#define NUM 16
int main()
{
  char cmd[SIZE];
  const char cmd_line[]="[myshell@VM-0-11-centos myshell]$";
  while(1)
  {
    cmd[0]='\0';
    printf("%s ",cmd_line);
    char *argv[NUM];
    fgets(cmd,SIZE,stdin);
    cmd[strlen(cmd)-1]='\0'; //去掉最后输入的\n，防止发生截取错误

    char *ptr=cmd;//获取重定向的文件名
    int redirect_type=-1;
    while(*ptr!='\0')
    {
      if(*ptr=='>')
      {
        ++redirect_type;
        *ptr='\0';
        ++ptr;
        if(*ptr=='>')
        {
          ++redirect_type;
         *ptr='\0';
          ++ptr;
        }
        break;
      }
      if(*ptr=='<')
      {
        *ptr='\0';
        redirect_type=2;
        ++ptr;
        break;
      }
      ++ptr;
    }
    if(*ptr!='\0')
    {
      while(isspace(*ptr))
      {
        ++ptr;
      }
    }
    else
    {
      ptr=NULL;
    }

    argv[0]=strtok(cmd," ");
    int i=1;
    while(1)
    {
      argv[i]=strtok(NULL," ");
      if( argv[i] ==NULL)
      {
        break;
      }
      ++i;
    }

    pid_t id=fork();
    if(id<0)
    {
      perror("fork error\n");
      continue;
    }
    if(id==0)
    {
      int fd;
      if(ptr!=NULL)
      {
        if(redirect_type==0)
        {
          fd=open(ptr,O_CREAT|O_TRUNC|O_WRONLY,0664);
          if(fd<0)
          {
            perror("open error");
            exit(2);
          }
          dup2(fd,1);
        }
        else if(redirect_type==1)
        {
          fd=open(ptr,O_CREAT|O_APPEND|O_WRONLY,0664);
          if(fd<0)
          {
            perror("open error");
            exit(2);
          }
          dup2(fd,1);
        }
        else if(redirect_type==2)
        {
          fd=open(ptr,O_RDONLY);
          if(fd<0)
          {
            perror("open error");
            exit(3);
          }
          dup2(fd,0);
        }
      }
      execvp(argv[0],argv);//进程替换
      exit(1);
    }
    int status=0;
    pid_t ret=waitpid(id,&status,0);  //父进程以阻塞的方式等待子进程
    if(ret>0)
    {
      if((status&0x7f)==0) //WIFEXITED
      {
        printf("child exit code:%d\n",(status>>8)&0xff); //WEXITED 查看子进程正常退出的退出码
      }
      else
      {
        printf("sign code:%d\n",status&0x7F);
      }
    }
  }
  return 0;
}
