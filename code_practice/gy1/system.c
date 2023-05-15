#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid
#include <errno.h> 


int main()
{
   int status; // 用于存储子进程的状态
 
   pid_t child = fork(); //forks make a copy of variables
   if (child < 0){
      perror("The fork calling was not succesful\n");
      exit(1);
   }

   if (child > 0){ //the parent process, it can see the returning value of fork - the child variable!
      waitpid(child, &status, 0); 
      printf("The end of parent process\n");
   }else{ //child process
      //to start a program, like printing out a string (in parameter) 5 times (parameter)
      printf("Call ./write program by calling system function\n");
      system("./write 'Operating Systems' 5");
      printf("It returns to continue child process after system call \nbecause it starts a new child process and in it will be called an execv\n");
   }

   return 0;
}

/*
   system() 函数在执行外部程序时，会启动一个新的子进程，并在其中调用 execv() 函数。
   与 execv() 不同，system() 函数在外部程序执行完成后会返回，允许子进程继续执行。
   因此，在这个例子中，子进程会在 system() 调用之后打印一条消息，表示外部程序已执行完成，子进程将继续执行。
*/