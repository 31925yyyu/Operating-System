#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid

/*
程序的主要部分包括：

1. handler 函数：这是一个自定义的信号处理函数，当指定的信号到达时会被调用。在这个例子中，它会打印收到的信号编号。

2. 父子进程间使用信号（signal）进行通信

    使用 signal 函数将自定义的 handler 函数与 SIGTERM 信号关联起来。当 SIGTERM 信号到达时，将调用 handler 函数。
    调用 fork() 创建一个子进程。
    对于父进程，调用 pause() 函数等待信号到达。当信号到达时，handler 函数会被调用。然后父进程等待子进程结束并打印 "Parent process ended"。
    对于子进程，等待3秒后使用 kill() 函数向父进程发送 SIGTERM 信号。然后打印 "Child process ended" 并退出。

*/

void handler(int signumber){
  printf("Signal with number %i has arrived\n",signumber);
}

int main(){

    // 当进程接收到SIGTERM信号时，会调用handler函数，显式地注册了handler函数来处理这个信号
    signal(SIGTERM, handler); //handler = SIG_IGN - ignore the signal (not SIGKILL,SIGSTOP), 
                              //handler = SIG_DFL - back to default behavior 
  
    pid_t child = fork();
    if(child < 0){
      perror("The fork calling was not succesful\n");
      exit(1);
    }

    if (child > 0){ // parent process

      pause(); //waits till a signal arrive 
      printf("Signal arrived\n", SIGTERM);
      
      int status;
      wait(&status);
      printf("Parent process ended\n");
    }else { //child process
      printf("Waits 3 seconds, then send a SIGTERM %i signal\n",SIGTERM);
      sleep(3);

      kill(getppid(), SIGTERM); 
      /*  
          getppid() 函数返回当前进程的父进程的进程ID。
          因此，这行代码向当前进程的父进程发送一个 SIGTERM 信号。
          当父进程接收到 SIGTERM 信号时，它将调用已注册的信号处理函数（在这个例子中是 handler 函数）来处理该信号。
      */
      //1. parameter the pid number of process, we send the signal
      // 		if -1, then eacho of the processes of the same uid get the signal
      // 		we kill our bash as well! The connection will close
      //2. parameter the name or number of signal
    printf("Child process ended\n");  
  }

    return 0;
}
