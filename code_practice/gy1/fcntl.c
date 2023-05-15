#include <stdlib.h>  
#include <stdio.h>
#include <sys/types.h> //fork-hoz
#include <sys/stat.h>
#include <unistd.h> //fork
#include <string.h> //strlen
#include <fcntl.h> //lock


/*

  父进程和子进程中并发(同时发生)地写入同一个文件 -- 通过文件锁来解决， fcntl() --- file control
  通过fcntl()设置文件锁来确保父进程和子进程在写入文件时不会相互干扰。
  文件锁确保了在一个进程写入文件时，另一个进程无法同时访问文件。

    F_GETFD（获取文件描述符的标志 F_SETFD（设置文件描述符的标志 F_GETFL（获取文件状态标志）
    F_SETFL（设置文件状态标志）F_GETLK（获取文件锁的信息）F_SETLK（尝试设置文件锁，但不等待）
    F_SETLKW（设置文件锁并等待，直到锁可用）等。
*/

int main()
{
    char text1[]=" Parent Parent Parent Parent Parent Parent Parent Parent Parent Parent ";
    int length1 = strlen(text1);
    char text2[] = " Child Child Child Child Child Child Child Child Child Child Child Child ";
    int length2 = strlen(text2);
  
    int i;
  
    struct flock lock_data; // lock structure 定义一个 flock 结构体 lock_data，用于存储文件锁的信息
    lock_data.l_whence = SEEK_SET;//the absolute starting position of locking设置锁定的起始位置为文件起始位置（SEEK_SET）
    // SEEK_SET - start of file文件起始位置, SEEK_END - end of file文件末尾位置, SEEK_CUR - actual place of file pointer cursor文件指针当前位置
    lock_data.l_start = 0 ; // relative position of locking to l_whence 表示锁的起始位置相对于l_whence
    lock_data.l_len = 0; // 表示锁定整个文件 how long is the locked part of the file, 0 - if it depends on the whole file 
    
    int rc = 0; //result of lock 
    int f;
    f = open("data.txt", O_RDWR|O_TRUNC|O_CREAT, S_IRUSR|S_IWUSR);
    // 使用 open() 函数以读写模式（O_RDWR）、截断模式（O_TRUNC）和创建模式（O_CREAT）打开（或创建）一个名为 "data.txt" 的文件，
    // 并设置其权限为用户可读写（S_IRUSR | S_IWUSR）。文件描述符存储在变量 f 中。
    
    pid_t child = fork();
    
    if (child<0){ //hiba
      perror("Error"); exit(1);
    }
    
    if (child>0)
    { //Parent process
      for (int j = 0; j < 25; j++) //to avoid too big data files
      { 
          // printf("szulo %i ",getpid());
          lock_data.l_pid = getpid();    //which process locks
          lock_data.l_type = F_WRLCK;    //for what it is locked F_WRLCK-write,FRDLCK-read,F_UNLCK - unlock
          rc = fcntl(f, F_SETLKW, &lock_data); //F_setlkw - locks file for writing and it is waiting, F_SETLK - it locks if it can, but not wait
          struct flock l2;
          if (rc!=(-1))
          {        
            for (i=0;i < length1;i++){
              write(f, &text1[i], 1);
              usleep(20);  //waits 20 milisec to let slow down writing 休眠20毫秒     
            }  
            write(f,"\n",1);
            lock_data.l_type = F_UNLCK;
            fcntl(f,F_SETLKW,&lock_data); //unlock file 释放文件锁
          }
      }

      int status;
      waitpid(child,&status,0); //wait for the end of child process
    }else 
    { //Child process
      for (int j = 0; j < 25; j++) //to avoid too big data file
      {
          lock_data.l_pid = getpid();
          lock_data.l_type = F_WRLCK;
          rc = fcntl(f,F_SETLKW,&lock_data); //lock file
          
          if (rc!=(-1))
          {       
              for (i=0;i < length2; i++)
              {
                  write(f, &text2[i], 1);
                  usleep(20);
              }  
            
              write(f,"\n",1);
              lock_data.l_type = F_UNLCK;
              fcntl(f, F_SETLKW, &lock_data);  //unlock file 释放文件锁 
          };
        }
      }
    
    close(f);
    return 0;
}
