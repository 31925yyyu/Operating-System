#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid


void handler(int signumber){
  printf("Signal with number %i has arrived\n",signumber);
}

int main(){
    struct sigaction sigact; 
    sigact.sa_handler = handler; // SIG_DFL,SIG_IGN 设置信号处理函数
    sigemptyset(&sigact.sa_mask); //during execution of handler these signals will be blocked plus the signal    初始化为包含所有信号的集合 
    //now only the arriving signal, SIGTERM will be blocked
    sigact.sa_flags = 0; //nothing special behaviour 将标志设置为 0，表示没有特殊行为
    sigaction(SIGTERM, &sigact, NULL);
    sigaction(SIGUSR1, &sigact, NULL);
    //1. parameter the signal number
    //2. parameter the new sigaction with handler and blocked signals during the execution of handler (sa_mask) and a 
    //special sa_flags - it change the behavior of signal, 
    //e.g. SIGNOCLDSTOP - after the child process ended it won't send a signal to the parent 
    //3. parameter - &old sigset or NULL. 
    //If there is a variable, the function will fill with the value of formerly set sigset
    pid_t child = fork();
    if (child>0)
    { // parent process
      sigset_t sigset; // a set of signals 创建一个信号集
      sigfillset(&sigset); //fill the set with all signals 填充信号集
      sigdelset(&sigset, SIGTERM); //delete SIGTERM from the set 删除 SIGTERM 信号
      int ret = sigsuspend(&sigset); // wait for a signal from the set 挂起当前进程，直到接收到信号集中的任何信号。等待除 SIGTERM 之外的任何信号
      // sigsuspend() 函数用于暂停进程，直到接收到指定信号集中的任何一个信号。当接收到信号并且信号处理函数完成执行后，sigsuspend() 返回。
      // like pause() - except it waits only for signals not given in sigset 后者会等待任何信号，而前者可以指定等待特定的信号集。
      //others will be blocked

      // 通过检查 errno 的值来判断 sigsuspend() 是否因为接收到信号而返回. 如果返回值为 -1，且 errno 的值为 EINTR，则表示 sigsuspend() 是因为接收到信号而返回的。
      if (ret == -1 && errno == EINTR)
      {
          printf("sigsuspend() returned due to a signal.\n");
      }

      printf("The program comes back from suspending\n");
      int status;
      wait(&status);
      printf("Parent process ended\n");
    }
    else 
    { //child process
      printf("Waits 3 seconds, then send a SIGUSR %i signal (it is not waited for by suspend) - so the suspend continues waiting\n", SIGUSR1);
      sleep(3);
      //kill(getppid(), SIGUSR1);
      printf("Waits 3 seconds, then send a SIGTERM %i signal (it is waited for by suspend)\n", SIGTERM);
      sleep(3);
      kill(getppid(), SIGTERM);
      printf("Child process ended\n");  
    }
    return 0;
} 