#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid

/*
  sigprocmask() 函数用于检查或修改一个进程的信号屏蔽字（signal mask）。 
  信号屏蔽字是一个信号集，它定义了哪些信号在进程中被阻塞。被阻塞的信号不会被传递到进程，而是保持挂起状态，直到解除阻塞。
*/


void handler(int signumber){
  printf("Signal with number %i has arrived\n",signumber);
}

int main(){

  sigset_t sigset;
  sigemptyset(&sigset); //empty signal set
  sigaddset(&sigset, SIGTERM); // SIGTERM is in set
  //sigfillset(&sigset); //each signal is in the set
  sigprocmask(SIG_BLOCK, &sigset, NULL); //signals in sigset will be blocked
  //parameters, how: SIG_BLOCK, SIG_UNBLOCK, SIG_SETMASK -   ; 
  //2. parameter changes the signalset to this if it is not NULL,
  //3.parameter if it is not NULL, the formerly used set is stored here
    
  signal(SIGTERM, handler); // signal and handler is connetcted
  signal(SIGUSR1, handler); //handler = SIG_IGN - ignore the signal (not SIGKILL,SIGSTOP), 
                           //handler = SIG_DFL - back to default behavior 
  
  pid_t child=fork();
  if (child>0)
  {
    /*
    在父进程中，调用pause()函数等待信号。由于SIGTERM被阻塞，父进程将只接收到SIGUSR1信号。
    调用sigprocmask(SIG_UNBLOCK, &sigset, NULL)来解除sigset中的信号阻塞。
    */
    pause(); // waits till a signal arrive SIGTERM is blocked, so it gets SIGUSR1
    printf("The blocking of SIGTERM %i signal is released, so the formerly sent SIGTERM signal arrives\n", SIGTERM); // 这是一个提示信息，表明即将解除SIGTERM信号的阻塞。
    
    // 解除sigset中的信号阻塞
    sigprocmask(SIG_UNBLOCK, &sigset, NULL);  //SIGTERM is now released (not blocked further), the process will get it
    // 由于SIGTERM在sigset中，这将解除SIGTERM信号的阻塞。解除阻塞后，先前发送但被阻塞的SIGTERM信号现在将传递给父进程。
    

    int status;
    wait(&status);
    printf("Parent process ended\n");
  }
  else 
  {
    printf("Waits 3 seconds, then send a SIGTERM %i signal (it is blocked)\n", SIGTERM);
    sleep(3);
    kill(getppid(), SIGTERM);
    printf("Waits 3 seconds, then send a SIGUSR1 %i signal (it is not blocked)\n", SIGUSR1);
    sleep(3);
    kill(getppid(), SIGUSR1);
    printf("Child process ended\n");  
  }
  return 0;
}