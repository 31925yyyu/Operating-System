#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

/* 在父进程和子进程之间使用共享内存（shared memory）来共享数据

    ftok()函数通过将文件的索引节点（inode）号与提供的整数组合，生成一个唯一的键（key）。这个键可用于创建和访问共享内存、信号量等IPC对象。

    shmget() --- 创建一个共享内存段 接受三个参数：

            key：一个由 ftok() 生成的唯一键，用于标识共享内存段。
            500：共享内存段的大小，以字节为单位。在这个例子中，共享内存段的大小设置为 500 字节。
            IPC_CREAT | S_IRUSR | S_IWUSR：用于控制共享内存段的访问权限和创建选项的标志位组合。在这里，IPC_CREAT 表示如果共享内存段不存在，则创建一个新的共享内存段。S_IRUSR 和 S_IWUSR 分别表示允许用户（拥有者）读取和写入共享内存段。
            shmget() 函数返回一个整数，称为共享内存标识符（sh_mem_id）。这个标识符用于后续对共享内存段的操作，如连接共享内存、断开共享内存、删除共享内存等。如果创建共享内存段失败，shmget() 函数将返回 -1。

    shmat() --- 将共享内存段附加到进程的地址空间 接受三个参数：
            
            sh_mem_id：共享内存标识符，用于标识要连接的共享内存段。
            NULL：共享内存段附加到进程地址空间的地址。如果将这个参数设置为 NULL，系统将自动选择一个合适的地址。
            0：用于控制共享内存段的访问权限和创建选项的标志位组合。在这里，0 表示不对共享内存段的访问权限和创建选项进行设置。
            shmat() 函数返回一个指针，指向共享内存段附加到进程地址空间的地址。如果连接共享内存段失败，shmat() 函数将返回 -1。

    shmdt() --- 将共享内存段从进程的地址空间分离 接受一个参数：
            
            s：指向共享内存段附加到进程地址空间的地址的指针。
            shmdt() 函数不返回任何值。
    
    shmctl() --- 控制共享内存段 接受三个参数：
                
            sh_mem_id：共享内存标识符，用于标识要控制的共享内存段。
            IPC_RMID：用于控制共享内存段的操作。IPC_RMID 表示删除共享内存段。
            NULL：用于控制共享内存段的访问权限和创建选项的标志位组合。在这里，NULL 表示不对共享内存段的访问权限和创建选项进行设置。
            shmctl() 函数不返回任何值。
*/
int main (int argc,char* argv[]) {

    int pid = fork();
    if(pid<0){
        perror("fork");
        exit(1);
    }
    char *s;

	// key creation
    key_t key; // 该键将用于创建和访问共享内存。
    int sh_mem_id; // shared memory id
    key = ftok(argv[0], 1);
    // create shared memory for reading and writing (500 bytes )
    sh_mem_id = shmget(key, 500, IPC_CREAT|S_IRUSR|S_IWUSR); // 创建一个共享内存段。该共享内存段的大小为 500 字节。
    // to connect the shared memory, 
    s = shmat(sh_mem_id, NULL, 0); // 共享内存区域附加到父进程和子进程的地址空间。这使它们能够访问共享内存。
    //
    
    if(pid>0){    
  
       char buffer[] = "Hurray Fradi! \n";                   
       // copy the value into the shared memory 将一段文本写入共享内存
       strcpy(s, buffer);
       
       shmdt(s);  // release the memory 使用shmdt()函数分离共享内存。这意味着父进程不再可以访问该内存区域。
        // s[0]='B';  //this causes a segmentation fault 
       wait(NULL); 
	// IPC_RMID- to clear the shared memory 删除共享内存
       shmctl(sh_mem_id, IPC_RMID, NULL);
    } else if ( pid == 0 ) 
	{
	    sleep(1);	              
        printf("Child: %s",s);
	    // it releases the shared memory 子进程不再可以访问该内存区域。
        shmdt(s);
	}

   return 0;
}
