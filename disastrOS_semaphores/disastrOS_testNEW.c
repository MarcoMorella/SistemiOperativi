#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_globals.h"

#define SEMNUM_FILL 0
#define SEMNUM_EMPTY 1
#define SEMNUM_ME1 2
#define SEMNUM_ME2 3
#define BUFFER_LENGTH 5
#define ITERATIONS 10
#define HOWMANY 10

#define ERROR_HELPER(cond, msg) do {    \
        if (cond && (running->pid!=0)) {              \
            printf("%s: %d \n", msg,running->pid);      \
              disastrOS_exit(disastrOS_getpid()+1); \
        }   \
        else if (cond && (running->pid==0)){     \
            printf("%s:",msg);  \
            disastrOS_exit(disastrOS_getpid()+1); \
        }   \
    } while(0)  \

void PrintBuffer(int * buffer){
    int i;
    printf("buffer : [ %d",buffer[0]);
    for(i = 1; i < BUFFER_LENGTH;i++){
        printf(" | %d",buffer[i]);}
    printf(" ]\n");
}

typedef struct child_data{
    int index_prod;
    int index_cons;
    int* c_buffer;
    }child_data;

void ProdFunction(void* args){
    int i,n,ret; //i : current Iteration   n : current block to work on   ret : return from syscalls

    printf("Starting producer with pid : %d\n",running->pid);
    //opening needed semaphores
    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_empty process ");

    int fd_me1 = disastrOS_semOpen(SEMNUM_ME1, 1);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_me1 process ");

    //structure containing buffer and index
    child_data* cd = (child_data*) args;

    //now let's use the semaphores to work the buffer
    for(i = 0;i < ITERATIONS;i++){
        ret = disastrOS_semWait(fd_empty);
        ERROR_HELPER(ret != 0, "Error semWait fd_empty process ");
        ret = disastrOS_semWait(fd_me1);
        ERROR_HELPER(ret != 0, "Error semWait fd_me1 process ");

        printf("Hello, i am prod and i am in CS! Pid : %d\n",running->pid);

        //using the index and advancing it
        n = cd->index_prod;
        cd->c_buffer[n] = running->pid;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_prod = n%BUFFER_LENGTH;


        ret = disastrOS_semPost(fd_me1);
        ERROR_HELPER(ret != 0, "Error semPost fd_me1 process ");

        ret = disastrOS_semPost(fd_fill);
        ERROR_HELPER(ret != 0, "Error semPost fd_fill process ");

    }

    //let's close everything,job's done
    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process");

    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process");

    ret = disastrOS_semClose(fd_me1);
    ERROR_HELPER(ret != 0, "Error semClose fd_me1 process");


    disastrOS_exit(disastrOS_getpid()+1);
}

void ConsFunction(void* args){
    int i,n,ret;

    printf("Starting consumer with pid : %d\n",running->pid);

    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH );
    ERROR_HELPER(fd_empty < 0,"Error semOpen fd_empty process ");

    int fd_me2 = disastrOS_semOpen(SEMNUM_ME2, 1);
    ERROR_HELPER(fd_me2 < 0,"Error semOpen fd_me2 process ");

    child_data* cd = (child_data*) args;

    for(i = 0;i < ITERATIONS;i++){

        ret = disastrOS_semWait(fd_fill);
        ERROR_HELPER(ret != 0, "Error semWait fd_fill process");

        ret = disastrOS_semWait(fd_me2);
        ERROR_HELPER(ret != 0, "Error semWait fd_me2 process ");

        printf("Hello,i am the cons and i am in CS! Pid : %d\n",running->pid);
        n = cd->index_cons;
        cd->c_buffer[n] = 0;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_cons = n%BUFFER_LENGTH;

        ret = disastrOS_semPost(fd_me2);
        ERROR_HELPER(ret != 0, "Error semPost fd_me2 process ");

        ret = disastrOS_semPost(fd_empty);
        ERROR_HELPER(ret != 0, "Error semPost fd_empty process ");

    }


    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");
    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");
    ret = disastrOS_semClose(fd_me2);
    ERROR_HELPER(ret != 0, "Error semClose fd_me2 process ");

    disastrOS_exit(disastrOS_getpid()+1);
}


void ProdFunction2(void* args){
    int n,ret;

    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_empty < 0,"Error semOpen fd_empty process ");

    //structure containing buffer and index
    child_data* cd = (child_data*) args;

    while (1){
        ret = disastrOS_semWait(fd_empty);
        ERROR_HELPER(ret != 0, "Error semWait fd_empty");

        n = cd->index_prod;
        cd->c_buffer[n] = running->pid;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_prod = n%BUFFER_LENGTH;

        ret = disastrOS_semPost(fd_fill);
        ERROR_HELPER(ret != 0, "Error semPost fd_fill ");
    }

    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");
    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");
}

void ConsFunction2(void* args){
    int n,ret;

    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_empty < 0,"Error semOpen fd_empty process ");

    //structure containing buffer and index
    child_data* cd = (child_data*) args;
    while (1){
        ret = disastrOS_semWait(fd_fill);
        ERROR_HELPER(ret != 0, "Error semWait fd_fill process");

        n = cd->index_cons;
        cd->c_buffer[n] = 0;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_cons = n%BUFFER_LENGTH;

        ret = disastrOS_semPost(fd_empty);
        ERROR_HELPER(ret != 0, "Error semPost fd_empty process ");
    }
    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");
    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");
}


void initFunction(void* args) {
    int i,ret;

    int c_buffer[BUFFER_LENGTH];   //populating the circular buffer
    for(i = 0;i < BUFFER_LENGTH;i++){
        c_buffer[i] = 0;}

    child_data cd;     //and the data structure that we'll give to the children
    cd.c_buffer = c_buffer;
    cd.index_prod = 0;
    cd.index_cons = 0;

    //let's open the semaphores in the father,so he'll unlink them at the end and there is no way children may close them totally
    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0, "Error semOpen fd_ill Father");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_empty < 0, "Error semOpen fd_mpty Father");

    int fd_me1 = disastrOS_semOpen(SEMNUM_ME1, 1);
    ERROR_HELPER(fd_me1 < 0, "Error semOpen fd_me1 Father");

    int fd_me2 = disastrOS_semOpen(SEMNUM_ME2, 1);
    ERROR_HELPER(fd_me2 < 0, "Error semClose fd_me2 Father");

    /* this is the 1 Producer 1 consumer test,with an infinite loop
    printf("1 producer,1 consumer\n");
    disastrOS_spawn(ProdFunction2,&cd);
    disastrOS_spawn(ConsFunction2,&cd);
    disastrOS_wait(0,NULL);
    disastrOS_wait(0,NULL);
    */

    printf("Spawning 10 Prods and 10 Cons\n");
    for (i = 0; i<HOWMANY; ++i) {
        disastrOS_spawn(ProdFunction,&cd);
        disastrOS_spawn(ConsFunction,&cd);

    }

    //waiting all the processes,in no particular order
    int pid_counter = 0;
    while(pid_counter < (2*HOWMANY)){
        disastrOS_wait(0,NULL);
        pid_counter++;
    }

    //closing the semaphores,father HAS to be the only one left using them

    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret!=0, "Error semClose fd_fill Father");
    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret!=0, "Error semClose fd_empty Father");
    ret = disastrOS_semClose(fd_me1);
    ERROR_HELPER(ret!=0, "Error semClose fd_me1 Father");
    ret = disastrOS_semClose(fd_me2);
    ERROR_HELPER(ret!=0, "Error semClose fd_me2 Father");

    printf("shutdown!\n");
    disastrOS_shutdown();
}

int main(int argc, char** argv){
    char* logfilename=0;
    if (argc>1) {
        logfilename=argv[1];
    }
    // we create the init process processes
    // the first is in the running variable
    // the others are in the ready queue
    //printf("the function pointer is: %p", childFunction);
    // spawn an init process
    printf("start\n");
    disastrOS_start(initFunction, 0, logfilename);
    return 0;
}
