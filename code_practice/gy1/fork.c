#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid
#include <errno.h> 


int main()
{
   int status;
   int notacommonvalue=1;
   printf("The value is %i before forking \n",notacommonvalue);
 
   pid_t child = fork(); //forks make a copy of variables
   
   if (child < 0){
      perror("The fork calling was not succesful\n"); 
      exit(1);
   } 

   if (child > 0) //the parent process, it can see the returning value of fork - the child variable!
   {
      waitpid(child, &status, 0);  // 当子进程结束后，waitpid可以修改status的值
      // waits the end of child process PID number = child, the returning value will be in status
      // 0 means, it really waits for the end of child process - the same as wait(&status)
      printf("The value is %i in parent process (remain the original) \n", notacommonvalue);
   }
   else //child process
   {
      notacommonvalue = 5; //it changes the value of the copy of the variable
      printf("The value is %i in child process \n", notacommonvalue);
   }
   return 0;
}


/*
这是一个 C 程序，演示了如何使用 fork() 系统调用来创建子进程。该程序还展示了 fork 的子进程如何获得父进程变量的副本，并可以在不影响父进程的情况下修改它们。

以下是代码的分解：

1. 包含头文件以提供所需的函数和数据类型。
2. 定义 main() 函数。
3. 在 main() 内部，初始化一个名为 notacommonvalue 的整数变量，值为 1。
4. 使用 printf() 输出 notacommonvalue 的值，表明分叉前的值。
5. 调用 fork() 函数创建子进程。fork() 返回值将被存储在 child 变量中。
6. 如果 fork() 返回值小于 0，表明分叉调用失败，打印错误信息并退出程序。
7. 如果 fork() 返回值大于 0，表明当前处于父进程中。父进程等待子进程结束，并在子进程结束后输出 notacommonvalue 的值（原始值未更改）。
8. 如果 fork() 返回值等于 0，表明当前处于子进程中。子进程修改 notacommonvalue 的值为 5，并输出修改后的值。

通过此程序，您可以了解到在父子进程中，变量是独立的。即使子进程修改了变量 notacommonvalue 的值，父进程中的 notacommonvalue 仍保持原值。
这是因为 fork() 在创建子进程时，复制了父进程的内存空间，因此子进程拥有父进程变量的副本。所以，当子进程修改变量值时，不会影响到父进程中的相应变量。

1. pid_t child = fork();
fork() 函数用于创建一个新的子进程。子进程是父进程的副本，拥有相同的内存映像、文件描述符等。fork() 函数在父进程和子进程中都有返回值。

在父进程中，fork() 返回新创建的子进程的进程 ID（一个大于 0 的整数），而在子进程中，fork() 返回 0。
pid_t 是一个数据类型，用于表示进程 ID。
在这行代码中，我们将 fork() 的返回值存储在一个名为 child 的 pid_t 类型的变量中。

2. waitpid(child, &status, 0);
waitpid() 函数用于让父进程等待子进程的结束。这个函数接受三个参数：
child：子进程的进程 ID。父进程通过 waitpid() 函数等待这个特定的子进程。
*/