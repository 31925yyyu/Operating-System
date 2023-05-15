#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <sys/types.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <unistd.h> 
#include <sys/wait.h>

struct messg { 
     long mtype;// this is a free value e.g for the address of the message
     char mtext [ 1024 ]; // this is the message itself
}; 

// sendig a message
int send( int mqueue ) 
{ 
     const struct messg m = { 5, "Hurray Fradi!"}; 
     int status; 
     
     status = msgsnd( mqueue, &m, strlen ( m.mtext ) + 1 , 0 ); 
	// a 3rd parameter may be : sizeof(m.mtext)
     	// a 4th parameter may be  IPC_NOWAIT, equivalent with  0-val 
     if ( status < 0 ) 
          perror("msgsnd error"); 
     return 0; 
} 
     
// receiving a message. 
int receive( int mqueue ) 
{ 
     struct messg m; 
     int status; 
     // the last parameter is the number of the message
	// if it is 0, then we read the first one
	// if  >0 (e.g. 5), then message type 5 will be read
	 
     status = msgrcv(mqueue, &m, 1024, 5, 0 ); 
     
     if ( status < 0 ) 
          perror("msgsnd error"); 
     else
          printf( "The code of the message is : %ld, text is:  %s\n", m.mtype, m.mtext ); 
     return 0; 
} 

int main (int argc, char* argv[]) { 
     pid_t child = fork(); 
     int messg, status; 
     key_t key; 
     
     // msgget needs a key, created by  ftok  
     //. 
     key = ftok(argv[0], 1); 
     printf ("The key: %d\n",key);
     messg = msgget( key, 0600 | IPC_CREAT ); 
     /*
          这行代码的作用是获取一个新的或现有的消息队列，并将其标识符存储在msgid变量中。
          如果指定的键对应的消息队列不存在，将创建一个具有可读可写权限的新消息队列。
          0600表示拥有者（owner）具有读和写权限，而其他用户（group和others）没有任何权限
     */
     if ( messg < 0 ) { 
          perror("msgget error"); 
          return 1; 
     } 
     
     if ( child > 0 ) { // Parent process.
          send( messg );  // Parent sends a message. 
          wait( NULL ); 
          // After terminating child process, the message queue is deleted. 
          status = msgctl( messg, IPC_RMID, NULL ); 
          if ( status < 0 ) 
               perror("msgctl error"); 
          
          printf("The parent process is terminated.\n");
          return 0; 
     } else if ( child == 0 ) {  // Child process.
          return receive( messg ); 
          // The child process receives a message. 
          printf("The child process is terminated.\n");
     } else { 
          perror("fork error"); 
          return 1; 
     } 
     
     return 0; 
} 
