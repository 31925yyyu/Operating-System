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
#include <sys/shm.h> //shared memory
#include <sys/types.h>//semaphor
#include <sys/stat.h> //S_IRUSR


/*
    Task: In the framework of Erasmus, one teacher (parent) goes on a study trip to London with 2 PHD students (children)
*/

/*
    Task1: After breakfast they discuss the plan for the day's trip. 
    Once the students (children's processes) are ready, they set off for the city. 
    On departure, they send a signal to the instructor who, after receiving the signal, sends via a pipe of where and when they will meet at the end of the day, after the day trip. 
    The time and place of the meeting is given as a parameter, e.g. a.out "Oxford Circus" 17. Students, having received the message, write it on a screen and terminal, parent waits for children to terminal.

*/

/*
    Task2:  One student arrives at the meeting place early and sees that there is a protest, very large crowd. 
    Therefore, on a message queue, he indicates a new location, for example: "Big crowd, in an hour at Trafalgar Square!" 
    He sends this to both the listener's partner and the instructor, who, having received the message, write it on a screen and then terminal it.
*/

/*
    Task3: . During the day trip, everyone makes a note of the places they have visited, 
    e.g. "Child1: British Museum, Tower, Big Ben", "Child2: Westminster, London Eye, Soho" or "Tutor: Piccadilly Circus, Buckingham Palace, Hyde Park". 
    After meeting, they share this information and places by writing them down in a shared memory. Each process adds its own list to the data already entered. Initially, the instructor enters this text into shared memory: 'Daily list:' (Null character at the end) After the child processes are terminated, the parent writes out from shared memory on the screen the daily attractions of all three, and then the parent is also terminated.
*/

/*
    Task4: Using shared memory is dangerous, so protect the operation with semaphore
*/

#define SEM_KEY 123456
#define SHM_KEY 654321
#define SHM_SIZE 4096

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

void handler(int sig){
    printf("Signal Received : I am ready!\n");
}

typedef struct{
    long msg_type;
    char msg_text[1024];
} message;

int main(int argc,char* argv[]){

    struct sigaction sigact;
    sigact.sa_handler = handler;
    sigact.sa_flags = 0;
    sigemptyset(&sigact.sa_mask);
    sigaction(SIGUSR1, &sigact, NULL);

    pid_t phd1_child, phd2_child;

    message msg;
    key_t key = ftok("erasmus.c", 65);
    int msgid;
    msgid = msgget(key, 0666 | IPC_CREAT);
    if(msgid == -1){
        perror("Message Queue Error");
        exit(1);
    }

    int pipefd[2];
    if(pipe(pipefd) == -1){
        perror("Pipe Error");
        exit(1);
    }

    key_t key_sem;
    key_sem = ftok("erasmus.c", 1);
    semid = semaphore_create("erasmus.c", 1);

    key_t key_shm;
    key_shm = ftok("erasmus.c", 2);
    shmid = shmget(key_shm, SHM_SIZE, 0600 | IPC_CREAT);
    if(shmid == -1){
        perror("shmget");
        exit(1);
    }
    char *shmaddr = shmat(shmid, NULL, 0);
    if (shmaddr == (char *)-1) {
        perror("shmat");
        exit(1);
    }
    strcpy(shmaddr, "Daily list:\n");

    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    phd1_child = fork();
    if(phd1_child == -1){
        perror("Fork Error");
        exit(1);
    }

    if(phd1_child == 0){ // phd child 1
        // send the signal to parent
        kill(getppid(), SIGUSR1);

        // Task1 : After breakfast they discuss the plan for the day's trip.
        char buf[1024];
        close(pipefd[1]); // close the write end of pipe
        read(pipefd[0], buf, 1024);
        printf("PHD1 Recieved: %s\n", buf);
        close(pipefd[0]);

        // Task2 : One student arrives at the meeting place early and sees that there is a protest, very large crowd.
        // Send message to message queue to both the listener's partner and the instructor

        msg.msg_type = 1;
        sprintf(msg.msg_text, "Big crowd, in an hour at Trafalgar Square!");
        int status = msgsnd(msgid, &msg, sizeof(msg), 0);
        if(status == -1){
            perror("Message Send Error");
            exit(1);
        }

        // Task3 and Task4: During the day trip, everyone makes a note of the places they have visited
        // Write to shared memory
        semaphore_lock(semid);
        strcat(shmaddr, "Child1: British Museum, Tower, Big Ben\n");
        semaphore_unlock(semid);

        exit(0);
    }

    phd2_child = fork();
    if(phd2_child == -1){
        perror("Fork Error");
        exit(1);
    }

    if(phd2_child == 0){ // phd child 2
        // send the signal to parent
        kill(getppid(), SIGUSR1);

        char buf[1024];
        close(pipefd[1]); // close the write end of pipe
        read(pipefd[0], buf, 1024);
        printf("PHD2 Recieved: %s\n", buf);
        close(pipefd[0]);

        // Task3: During the day trip, everyone makes a note of the places they have visited
        // Write to shared memory
        semaphore_lock(semid);
        strcat(shmaddr, "Child2: London Eye, Hyde Park\n");
        semaphore_unlock(semid);

        exit(0);
    }



    // parent
    close(pipefd[0]); // close the read end of pipe
    sigsuspend(&oldmask);  // 使用sigsuspend代替pause
    sigsuspend(&oldmask);

    sigprocmask(SIG_UNBLOCK, &mask, NULL);
    char buf[1024];
    sprintf(buf, "We are planning to meet at %s at %s.", argv[1], argv[2]);
    write(pipefd[1], buf, strlen(buf)+1);  // write message for first child
    sleep(2);
    write(pipefd[1], buf, strlen(buf)+1);  // write message for second child
    close(pipefd[1]);
    
    // Task2 : One student arrives at the meeting place early and sees that there is a protest, very large crowd.
    // Receive message from message queue to both the listener's partner and the instructor
    if(msgrcv(msgid, &msg, sizeof(msg), 1, 0) == -1){
    perror("Message Receive Error");
    exit(1);
    }
    printf("Tutor Message Received: %s\n", msg.msg_text);
    sleep(2);  // Add this line to allow the parent process to receive the message before PHD1 exits
    msgctl(msgid, IPC_RMID, NULL); // Delete the message queue

    wait(NULL);
    wait(NULL);

    // Add visited places to shared memory
    semaphore_lock(semid);
    strcat(shmaddr, "Tutor: Piccadilly Circus, Buckingham Palace, Hyde Park\n");
    semaphore_unlock(semid);

    // Print the contents of the shared memory after all child processes have terminated
    printf("%s", shmaddr);

    // Clean up the shared memory and semaphore before the parent process terminates
    if (shmdt(shmaddr) == -1) {
        perror("shmdt");
        exit(1);
    }
    if (shmctl(shmid, IPC_RMID, NULL) == -1) {
        perror("shmctl");
        exit(1);
    }
    semaphore_delete(semid);

    return 0;
}
