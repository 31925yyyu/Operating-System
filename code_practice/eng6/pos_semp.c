//*********************************
// Compile with -pthread 
//*****************************

// 使用 POSIX 信号量和共享内存进行进程间同步

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MEMSIZE 1024 // 设置共享内存区域的大小

// 创建一个信号量
sem_t* semaphore_create(char *name, int semaphore_value) // 接收一个信号量名称和初始值
{
	// 使用 sem_open 函数创建一个新的信号量，并将其权限设置为可读可写。
    sem_t *semid = sem_open(name, O_CREAT,S_IRUSR|S_IWUSR, semaphore_value);
	if (semid == SEM_FAILED){
		perror("sem_open");
	}
       
    return semid;
}

// 删除一个信号量
void semaphore_delete(char* name)
{
	sem_unlink(name); // 删除指定名称的信号量
}

int main (int argc,char* argv[]) {

    pid_t child;
    key_t key;
    int sh_mem_id;
    sem_t* semid;
    char *s;
    char* sem_name = "/apple";

    key = ftok(argv[0], 1); // 生成一个键值,用于共享内存和信号量的创建
    sh_mem_id = shmget(key, MEMSIZE,IPC_CREAT|S_IRUSR|S_IWUSR); // 创建一个共享内存区域,权限设置为可读可写
    s = shmat(sh_mem_id, NULL, 0); // 将共享内存段附加到进程的地址空间

    semid = semaphore_create(sem_name, 0); // 创建名为 /apple 的信号量，初始值为 0。

    child = fork();
	if(child<0){
		perror("fork");
		exit(1);
	}

    if(child > 0)	// parent process
	{    
       	char buffer[] = "I like Illes pop group!\n";
		printf("Parent starts and writes into the shared memory: %s\n", buffer);

       	sleep(4); // wait for a few seconds 等待4秒，让子进程有时间等待信号量  

       	strcpy(s, buffer);	// copy data to shared memory 将字符串 buffer 复制到共享内存区域。
		printf("Parent: semaphore up!\n");
       	sem_post(semid);	// semaphore up 信号量加 1
		shmdt(s);	// release shared memory 释放共享内存区域

       	wait(NULL); // wait for child process to terminate 等待子进程终止

       	semaphore_delete(sem_name); // delete semaphore 删除信号量
		shmctl(sh_mem_id, IPC_RMID, NULL); // delete shared memory 删除共享内存区域
    } else if ( child == 0 ){ //child
			
       	/*  critical section   */
		printf("Child tries to close semaphore!\n");
       	sem_wait(semid);	// semaphore down 信号量减1, 如果信号量的值为0,则阻塞等待
       	printf("Child: the result is: %s", s);  
       	sem_post(semid); // semaphore up 信号量加 1
       	/*  end of critical section  */  	  
       	shmdt(s); // release shared memory 释放共享内存区域
    }

   return 0;
}


