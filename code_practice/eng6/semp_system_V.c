#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

// System V 信号量和共享内存实现进程间同步
// 当一个进程需要等待另一个进程完成某个任务后再进行下一步操作时，我们需要实现进程间的同步

#define MEMSIZE 1024 // size of shared memory

// create a semaphore
int semaphore_create(const char* pathname, int semaphore_value){
    int semid; // semaphore id
    key_t key; // semaphore key
    
    key=ftok(pathname, 1);    
    if((semid = semget(key, 1, IPC_CREAT|S_IRUSR|S_IWUSR ))<0)
	perror("semget");
    // semget 2. parameter is the number of semaphores   
    if(semctl(semid,0,SETVAL,semaphore_value)<0)    //0= first semaphores
        perror("semctl");
       
    return semid;
}

// semaphore operation
void semaphore_operation(int semid, int op){ // op=1 up, op=-1 down  接收一个信号量标识符和操作值

    struct sembuf operation; // sembuf结构体用于存储信号量操作
    
    operation.sem_num = 0;  // 0 = first semaphores 第一个信号量
    operation.sem_op  = op; // op=1 up, op=-1 down 
    operation.sem_flg = 0; // 0 = wait if necessary 等待信号量可用
    
    if(semop(semid,&operation,1)<0) // 1 number of sem. operations
        perror("semop");	    
}

// delete a semaphore
void semaphore_delete(int semid){
      semctl(semid,0,IPC_RMID);
}

int main (int argc, char* argv[]) {

    pid_t child;
    key_t key;
    int sh_mem_id, semid;
    char *s;

    key = ftok(argv[0], 1); // generate a key for shared memory and semaphore
    sh_mem_id = shmget(key, MEMSIZE, IPC_CREAT|S_IRUSR|S_IWUSR); // create shared memory 创建共享内存段
    s = shmat(sh_mem_id, NULL, 0); // attach shared memory to process address space 将共享内存段附加到进程的地址空间

    semid = semaphore_create(argv[0], 0); // sem state is down!!! 创建信号量，初始值为 0（表示信号量状态为 down）信号量的值为 0 时，进程将被阻塞，直到信号量的值大于 0 为止。

    child = fork();
    if(child<0) perror("fork");

    if(child > 0){ // parent process  
        
        char buffer[] = "I like Illes (Pop group:)!\n";
        printf("Parent starts, writes the text into shared memory %s\n", buffer);
        sleep(4); // child waits during sleep
        strcpy(s, buffer); // copy text into shared memory 

        printf("Parent:, semaphore up!\n");
        semaphore_operation(semid, 1); // Up
        shmdt(s);	// release shared memory
        wait(NULL); // wait for child process

        semaphore_delete(semid);
        shmctl(sh_mem_id,IPC_RMID,NULL);
    } else if ( child == 0 ) {	
    
       // critical section
       printf("Child: try to close semaphore !\n");
       semaphore_operation(semid, -1); // down, wait if necessary 如果信号量值为 0，子进程将阻塞等待。

       // 当父进程将信号量值加 1 时，子进程继续执行并从共享内存中读取数据。

       printf("Child: result: %s",s);  
       semaphore_operation(semid, 1); // up      
       // end of critical section  
       shmdt(s);
    }

   return 0;
}


