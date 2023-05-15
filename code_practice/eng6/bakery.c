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

/*
	这个代码示例演示了如何使用 POSIX 信号量和共享内存实现一个简单的面包店模型。
	在这个模型中，有一个面包师傅（baker）生产面包，并将生产的面包放置在货架上。
	同时，有一个顾客（customer）从货架上取走面包。
	面包师傅和顾客需要通过信号量进行同步，以确保货架上的面包数量在一个合理的范围内。
*/

// posix semaphor requires to link: cc -pthread bakery.c
#define MEMSIZE 1024 // 设置共享内存区域的大小

int *s; // 共享内存区域
// 信号量
sem_t* free_;  // free is a C function
sem_t* empty;
sem_t* full;

// 创建一个信号量
sem_t* semaphore_create(char*name,int semaphore_value)
{
    sem_t *semid = sem_open(name,O_CREAT,S_IRUSR|S_IWUSR,semaphore_value );
	if (semid==SEM_FAILED)
	perror("sem_open");
    return semid;
}

// 删除一个信号量
void semaphore_delete(char* name)
{
      sem_unlink(name);
}

void baker()
{
	long int li;
	while(s[1]) // 面包师傅会不断地生产面包，直到 s[1] 为 0
	{
		sem_wait(empty); // 等待 empty 信号量
		sem_wait(free_); // 等待 free 信号量
		s[0] += 1; // 生产一个面包
		printf("Baker: the number of breads: %i\n", s[0]);
		sem_post(free_); // 释放 free 信号量
		sem_post(full); // 释放 full 信号量
		li = random(); // 产生一个随机数
		usleep(li % 100000); // 等待 0-100000 微秒 wait 0-100000 microsecond
		//	printf("Baker: I made a bread!\n");
	}
}

void customer()
{
	long int li;
	while(s[1]) // 顾客会不断地取走面包，直到 s[1] 为 0
	{
		sem_wait(full); // 在取走面包之前，顾客需要等待 full 信号量，以确保货架上有面包
		sem_wait(free_); // 顾客取走面包之后，需要等待 free 信号量，以确保货架上的面包数量在一个合理的范围内
		s[0] -= 1; // 取走一个面包 
		printf("Customer, The number of breads: %i\n",s[0]);
		sem_post(free_); // 取走面包后，释放 free 信号量
		sem_post(empty); // 并释放 empty 信号量，表明货架上有空位了
		li = random();
		usleep(li%100000);	// wait for a while, 0-100000 microsecond
		//	printf("Customer: I ate 1 bread!\n");
	}
}

int main (int argc,char* argv[]) {

    pid_t baker_id, customer_id; // create two processes -- baker and customer
    int sh_mem_id, N=10; // N is the size of the shelf and sh_mem_id is the id of the shared memory
    char* sem_name1 = "one";
    char* sem_name2 = "two";
    char* sem_name3 = "three";

	key_t key; // key is used to create the shared memory
    key = ftok(argv[0], 1); 
    sh_mem_id = shmget(key, MEMSIZE, IPC_CREAT|S_IRUSR|S_IWUSR); // create the shared memory
    s = (int*)shmat(sh_mem_id, NULL, 0); // 将共享内存段附加到进程的地址空间 
    s[0] = 0; 	// the number of breads on the shelf
    s[1] = 1;	// the process may start
    free_ = semaphore_create(sem_name1, 1);
    empty = semaphore_create(sem_name2, N);	// shelf is empty
    full = semaphore_create(sem_name3, 0);
    customer_id = fork();
    if(customer_id > 0){  // parent process
  		baker_id = fork(); // create baker process
		if (baker_id > 0){  // parent process
       		char buffer[100];
			printf("Parent starts! \n");
			printf("Press down something!\n");
			scanf("%s", &buffer);
			s[1] = 0;	// chid's end
			wait(NULL);
			wait(NULL);
			shmdt(s); // 分离共享内存
			shmctl(sh_mem_id, IPC_RMID, NULL); // delete the shared memory
       		semaphore_delete(sem_name1);
       		semaphore_delete(sem_name2);
       		semaphore_delete(sem_name3);
		}else{ // baker child process
		
			srandom(1000);	// init random generator in baker
			printf("Baker starts!\n");
			baker();
			printf("Baker ended!\n");
			shmdt(s);
		}
    } else {	// customer
		//srandom(10);	// init random generator in customer
		srandom(time(NULL));
		printf("Customer starts!\n");
       	customer();
       	printf("Customer ended\n");  
        shmdt(s);
    }

   return 0;
}


