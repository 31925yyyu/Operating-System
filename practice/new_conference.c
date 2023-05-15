#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h> //signal
#include <sys/msg.h> //message queue
#include <errno.h>   //perror
#include <unistd.h>  //fork
#include <sys/ipc.h>   //semaphor
#include <sys/wait.h>  //wait
#include <sys/sem.h> //semaphor
#include <sys/types.h>//semaphor
#include <sys/stat.h> //S_IRUSR
#include <sys/shm.h>

/*
    Starts the actual daily corona virus news conference 
    where the Communication Officer (parent process), the Police Lieutenant-colon (child) and the National Head Physician  (child) 
    take part and answer the press questions.  The Communication Officer commandeers the event and waits all of the answers. 
    (After all the parent waits the end of children and all along keeps the contact with them.) 
*/

/*
    Task1: At the beginning the Communication Officer (parent) waits since both of the other two members (children) to settle down 
    on the podium and they nod to him (send a signal) that they are ready to start. 
    (Write on the console) After it the Communication Officer asks the first question (using pipe) to the Police Lieutenant-colon - 
    "Is it compulsory to wear a mask in the shops?" He answers (through pipe) - "Yes, it is compulsory to wear the mask when you leave your flat!"  
*/

/*
    Task2: 2, After this the National Head Physician remarked "To wear a mask is really very important to save other people and ourself  
    against the virus in the shops and on the roads" (this remark is sent to the Communication Officer through a message queue)  .  
    The Communication Officer writes it out to the console.
*/

/*
    Task3: 3, Now the National Head Physician inform the audience about the number of new infected people. 
    It is a freely given number which she writes into the shared memory. The Communication Officer read it and write it to the console.
*/

/*
    Task4: Use semaphore to save the usage of shared memory! The parent should wait the end of children and then it terminates. 
*/

void handler(int sig){
    printf("Signal Received : We are ready!\n");
}


typedef struct{
    long msg_type;
    char msg_text[1024];
} message;

int *num_infected;
int shmid;
int semid;

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

void semaphore_delete(int semid){
    if(semctl(semid,0,IPC_RMID)<0)
        perror("semctl");
}

void semaphore_lock(int semid){
    struct sembuf sem_lock;
    sem_lock.sem_num = 0;
    sem_lock.sem_op = -1;
    sem_lock.sem_flg = 0;
    semop(semid, &sem_lock, 1);
}

void semaphore_unlock(int semid){
    struct sembuf sem_unlock;
    sem_unlock.sem_num = 0;
    sem_unlock.sem_op = 1;
    sem_unlock.sem_flg = 0;
    semop(semid, &sem_unlock, 1);
}


int main(){
    // Create two children
    pid_t police_child, physician_child;

    // Create a signal for the children to send to the parent
    struct sigaction sigact;
    sigact.sa_handler = handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGUSR1, &sigact, NULL);

    // create pipe
    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("pipe");
        exit(1);
    }

    police_child = fork();
    if(police_child < 0){
        perror("fork");
        exit(1);
    }

    if(police_child == 0){ // Police Lieutenant-colon child process
        // send signal to parent
        kill(getppid(), SIGUSR1);

        close(pipefd[0]); // close read end
        char *msg = "Yes, it is compulsory to wear the mask when you leave your flat!";
        write(pipefd[1], msg, strlen(msg)+1);
        close(pipefd[1]); // close write end
        exit(0);
    }

    // Task2 -- Create a message queue
    message messg;
    key_t key1;
    key1 = ftok("news_conference.c", 1);
    int msgid;
    msgid = msgget(key1, 0600 | IPC_CREAT);
    if(msgid == -1){
        perror("msgget");
        exit(1);
    }

    physician_child = fork();
    if(physician_child < 0){
        perror("fork");
        exit(1);
    }

    key_t key_shm;
    key_shm = ftok("news_conference.c", 2);
    shmid = shmget(key_shm, sizeof(int), 0600 | IPC_CREAT);
    if(shmid == -1){
        perror("shmget");
        exit(1);
    }
    num_infected = (int *)shmat(shmid, NULL, 0);

    key_t key_sem;
    key_sem = ftok("news_conference.c", 3);
    semid = semget(key_sem, 1, 0600 | IPC_CREAT);
    if(semid == -1){
        perror("semget");
        exit(1);
    }

    if(physician_child == 0){ // National Head Physician child process
        // send signal to parent
        kill(getppid(), SIGUSR1);

        // Task2 -- Send message to Communication Officer
        messg.msg_type = 1;
        strcpy(messg.msg_text, "To wear a mask is really very important to save other people and ourself against the virus in the shops and on the roads");
        if(msgsnd(msgid, &messg, sizeof(messg), 0) == -1){
            perror("msgsnd");
            exit(1);
        }

        // Task3 -- Write number of infected to shared memory
        sleep(2);
        *num_infected = 1000;
        shmdt(num_infected);

        // Task4 -- Use semaphore to save the usage of shared memory
        semctl(semid, 0, SETVAL, 1);


        exit(0);
    }

    // wait for both children to send signal
    sigset_t sigset;
    sigemptyset(&sigset);
    sigaddset(&sigset, SIGUSR1);
    sigprocmask(SIG_BLOCK, &sigset, NULL);
    while(1){
        sigsuspend(&sigset);
        sigprocmask(SIG_UNBLOCK, &sigset, NULL);
        break;
    }

    // Task1 -- Ask question to Police Lieutenant-colon(by pipe)
    printf("Communication Officer: Is it compulsory to wear a mask in the shops?\n");
    // read from pipe
    close(pipefd[1]); // close write end
    char buf[1024];
    read(pipefd[0], buf, sizeof(buf));
    printf("Police Lieutenant-colon: %s\n", buf);
    close(pipefd[0]); // close read end

    // Task2 -- Read message from National Head Physician
    if(msgrcv(msgid, &messg, sizeof(messg), 1, 0) == -1){
        perror("msgrcv");
        exit(1);
    }
    printf("National Head Physician: %s\n", messg.msg_text);

    // wait for children to finish
    wait(NULL);
    wait(NULL);

    // remove message queue
    if(msgctl(msgid, IPC_RMID, NULL) == -1){
        perror("msgctl");
        exit(1);
    }

    // Task3 -- Read number of infected from shared memory
    int num = *num_infected;
    printf("Communication Officer: Number of infected: %d\n", num);
    shmdt(num_infected);
    shmctl(shmid, IPC_RMID, NULL);

    // Task4 -- Use semaphore
    printf("Parent: Lock the semaphore\n");
    semaphore_lock(semid);
    sleep(2);
    printf("Parent: Unlock the semaphore\n");
    semaphore_unlock(semid);
    semaphore_delete(semid);
    shmctl(shmid, IPC_RMID, NULL);

    return 0;
}
