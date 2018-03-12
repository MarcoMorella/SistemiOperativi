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
        printf(" | %d",buffer[i]);
    }
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
    int ind = 0;
    while (ind < ITERATIONS * 30){
        ret = disastrOS_semWait(fd_empty);
        ERROR_HELPER(ret != 0, "Error semWait fd_empty process ");

        n = cd->index_prod;
        cd->c_buffer[n] = running->pid;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_prod = n%BUFFER_LENGTH;

        ret = disastrOS_semPost(fd_fill);
        ERROR_HELPER(ret != 0, "Error semPost fd_fill ");

        ind++;
    }

    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");

    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");

    disastrOS_exit(disastrOS_getpid()+1);
}

void ConsFunction2(void* args){
    int n,ret;

    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_empty < 0,"Error semOpen fd_empty process ");

    //structure containing buffer and index
    child_data* cd = (child_data*) args;
    int ind = 0;
    while (ind < ITERATIONS * 30){
        ret = disastrOS_semWait(fd_fill);
        ERROR_HELPER(ret != 0, "Error semWait fd_fill process ");

        n = cd->index_cons;
        cd->c_buffer[n] = 0;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_cons = n%BUFFER_LENGTH;

        ret = disastrOS_semPost(fd_empty);
        ERROR_HELPER(ret != 0, "Error semPost fd_empty process ");

        ind++;
    }

    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");

    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");

    disastrOS_exit(disastrOS_getpid()+1);
}


void ConsSingleFunction(void* args){
   int i,n,ret;

    printf("Starting consumer with pid : %d\n",running->pid);

    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH );
    ERROR_HELPER(fd_empty < 0,"Error semOpen fd_empty process ");


    child_data* cd = (child_data*) args;

    for(i = 0;i < ITERATIONS * HOWMANY;i++){
        ret = disastrOS_semWait(fd_fill);
        ERROR_HELPER(ret != 0, "Error semWait fd_fill process ");

        printf("Hello,i am the cons and i am in CS! Pid : %d\n",running->pid);
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

    disastrOS_exit(disastrOS_getpid()+1);
}

void ProdSingleFunction(void* args){
    int i,n,ret;

    printf("Starting producer with pid : %d\n",running->pid);

    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_fill process ");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_fill < 0,"Error semOpen fd_empty process ");

    child_data* cd = (child_data*) args;

    for(i = 0;i < ITERATIONS * HOWMANY;i++){
        ret = disastrOS_semWait(fd_empty);
        ERROR_HELPER(ret != 0, "Error semWait fd_empty process ");

        printf("Hello, i am prod and i am in CS! Pid : %d\n",running->pid);

        n = cd->index_prod;
        cd->c_buffer[n] = running->pid;
        PrintBuffer(cd->c_buffer);

        n++;
        cd->index_prod = n%BUFFER_LENGTH;

        ret = disastrOS_semPost(fd_fill);
        ERROR_HELPER(ret != 0, "Error semPost fd_fill process ");
    }

    ret = disastrOS_semClose(fd_fill);
    ERROR_HELPER(ret != 0, "Error semClose fd_fill process ");

    ret = disastrOS_semClose(fd_empty);
    ERROR_HELPER(ret != 0, "Error semClose fd_empty process ");

    disastrOS_exit(disastrOS_getpid()+1);
}



void initFunction(void* args) {
    int i,ret,Input;
    //Luke..I am your fath-..Processes,I am your father!
    printf("Hello there!We are going to test our Semaphores.\nThere are 4 possible tests right now:\n");
    printf("1)One consumer,one producer,looping until it is manageable.\n2)Multiple consumers,multiple producers,working a fixed number of iterations.\n");
    printf("3)One consumer,multiple producers,working a fixed number of iterations.\n4)Multiple consumers,one producer,working a fixed number of iterations.\nPlease insert the number of the test you want to execute: ");

    scanf("%d",&Input);

    int c_buffer[BUFFER_LENGTH];   //populating the circular buffer
    for(i = 0;i < BUFFER_LENGTH;i++){
        c_buffer[i] = 0;
    }

    child_data cd;     //and the data structure that we'll give to the children
    cd.c_buffer = c_buffer;
    cd.index_prod = 0;
    cd.index_cons = 0;

    //let's open the semaphores in the father,so he'll unlink them at the end and there is no way children may close them totally
    int fd_fill= disastrOS_semOpen(SEMNUM_FILL, 0);
    ERROR_HELPER(fd_fill < 0, "Error semOpen fd_fill Father");

    int fd_empty = disastrOS_semOpen(SEMNUM_EMPTY, BUFFER_LENGTH);
    ERROR_HELPER(fd_empty < 0, "Error semOpen fd_empty Father");

    int fd_me1 = disastrOS_semOpen(SEMNUM_ME1, 1);
    ERROR_HELPER(fd_me1 < 0, "Error semOpen fd_me1 Father");

    int fd_me2 = disastrOS_semOpen(SEMNUM_ME2, 1);
    ERROR_HELPER(fd_me2 < 0, "Error semClose fd_me2 Father");

    // this is the 1 Producer 1 consumer test,with an infinite loop
    if(Input == 1){
        printf("1 producer,1 consumer\n");
        disastrOS_spawn(ProdFunction2,&cd);
        disastrOS_spawn(ConsFunction2,&cd);
        disastrOS_wait(0,NULL);
        disastrOS_wait(0,NULL);
    }

    else if (Input == 2){
    //printf("Spawning 10 Prods and 10 Cons\n");
        for (i = 0; i<HOWMANY; ++i) {
            disastrOS_spawn(ProdFunction,&cd);
            disastrOS_spawn(ConsFunction,&cd);
        }


        //waiting all the processes,in no particular order
        for (i = 0; i<2*HOWMANY; ++i) disastrOS_wait(0,NULL);

    }

    else if(Input == 3){ //1 consumer , N producer
        for(i = 0;i<HOWMANY; ++i) {
            disastrOS_spawn(ProdFunction,&cd);
        }
        disastrOS_spawn(ConsSingleFunction,&cd);

        for(i = 0; i < HOWMANY+1; ++i){
            disastrOS_wait(0,NULL);
        }
    }

    else if(Input == 4){ //N consumer, 1 producer
        for(i = 0;i<HOWMANY; ++i) {
            disastrOS_spawn(ConsFunction,&cd);
        }
        disastrOS_spawn(ProdSingleFunction,&cd);

        for(i = 0; i < HOWMANY+1; ++i){
            disastrOS_wait(0,NULL);
        }

    }

    else{
        printf("Unrecognized case...shutting down!\n");
        disastrOS_shutdown();
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

void fun(void* args){

  int* fd = (int*) args;
  int ret;

  ret = disastrOS_semPost(*fd); //post on semaphore that exists but it's not open in this process using its descriptor
  printf("%d\n", ret);

  ret = disastrOS_semWait(*fd); //wait on semaphore that exists but it's not open in this process using its descriptor
  printf("%d\n", ret);

  ret = disastrOS_semClose(*fd); //close on semaphore that exists but it's not open in this process using its descriptor
  printf("%d\n", ret);

  ret = disastrOS_semPost(7); //post on semaphore that exists but it's not open in this process using its semnum
  printf("%d\n", ret);

  ret = disastrOS_semWait(7); //wait on semaphore that exists but it's not open in this process using its semnum
  printf("%d\n", ret);

  ret = disastrOS_semClose(7); //close on semaphore that exists but it's not open in this process using its semnum
  printf("%d\n", ret);


  disastrOS_exit(0);

}

void wrongInputs(void* args){
  int ret;
  ret = disastrOS_semPost(6); //post on semaphore that don't exists
  printf("%d\n", ret);
  ret = disastrOS_semWait(6); //wait on semaphore that don't exists
  printf("%d\n", ret);
  ret = disastrOS_semClose(6); //close on semaphore that don't exists
  printf("%d\n", ret);

  ret = disastrOS_semOpen(7, 1); //normal open
  printf("%d\n", ret);

  disastrOS_spawn(fun, &ret);

  disastrOS_wait(0,NULL);

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
    printf("DisastrOS starting!\n");
    //disastrOS_start(initFunction, 0, logfilename);
    disastrOS_start(initFunction, 0, logfilename);
    return 0;
}
