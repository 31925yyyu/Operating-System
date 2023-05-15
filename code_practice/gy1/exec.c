#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>  //fork
#include <sys/wait.h> //waitpid
#include <errno.h> 


int main()
{
   int status;
 
   pid_t  child=fork(); //forks make a copy of variables
   if (child<0){perror("The fork calling was not succesful\n"); exit(1);} 
 
   if (child > 0) //the parent process, it can see the returning value of fork - the child variable!
   {
      waitpid(child,&status,0); 
      printf("The end of parent process\n");
   }
   else //child process
   {
      //to start a program, like printing out a string (in parameter) 5 times (parameter)
      char * cmd="./write";
      char * arg[] = {"./write", "Operating Systems", "5", NULL}; 
      printf("./write program will be called by execv function\n");
      execv(cmd, arg);
      printf("It never returns to child process (except on error) - the content of the whole child process will be changed to the new one");
   }
   return 0;
}

/*
This C code demonstrates the usage of the fork(), waitpid(), and execv() system calls to create a child process that executes another program. Here is a step-by-step explanation:

1. Header files are included:
stdio.h: Standard input/output functions (e.g., printf)
stdlib.h: General-purpose functions (e.g., exit)
unistd.h: Provides the fork() function
sys/wait.h: Provides the waitpid() function
errno.h: Defines error numbers (used by perror)

2. The main() function is defined.

3. The int status variable is declared for storing the child process termination status.

4. The fork() function is called, creating a new child process. The return value of fork() is stored in the child variable.

-- If child < 0, fork() was not successful, and an error message is displayed using perror(). The program then exits with a status code of 1.
-- If child > 0, the current process is the parent process. The parent process waits for the child process to terminate by calling waitpid(child, &status, 0). 
After the child process has finished, the parent process prints "The end of parent process\n" and continues to the end of main().
-- If child == 0, the current process is the child process. In this case, the following steps are performed:

-- The cmd variable is set to the string "./write", which is the name of the program to be executed.
-- The arg variable is an array of strings that contains the arguments to be passed to the program. The first argument is the program name ("./write"), followed by the string "Operating Systems", the number "5", and a NULL pointer to indicate the end of the arguments.
-- The printf() function is used to print a message indicating that the "./write" program will be called by the execv() function.
-- The execv() function is called with cmd and arg. This function replaces the current child process's memory image with the new program specified by cmd and passes the arguments from arg. If execv() is successful, the child process will never return to the lines of code after it.

5. If execv() fails, the program flow will continue to the next line. The printf() function will print a message indicating that the control should never return to the child process (unless there's an error). 
This is because the execv() function replaces the current process's memory image with the new program, and the lines after the execv() call should not be executed unless there's an error.
execv() function 在成功执行后不会返回，因为子进程的内容被替换为新程序的内容

6. The main() function returns 0, indicating successful execution.

In summary, this code demonstrates how to create a child process using fork(), wait for the child process to finish using waitpid(), 
and replace the child process's memory image with a new program using execv(). 
The new program, "./write", will be executed with the arguments "Operating Systems" and "5".
*/