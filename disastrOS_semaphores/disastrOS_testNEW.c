#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_globals.h"



void ProdFunction(void* args){
  printf("%d YEEE\n",running->pid);
  disastrOS_exit(disastrOS_getpid()+1);
}

void ConsFunction(void* args){
  printf("%d OOOOW\n",running->pid);

  disastrOS_exit(disastrOS_getpid()+1);
}


void initFunction(void* args) {

    int c_buffer[5];   //populating the circular buffer
    int i;
    for(i = 0;i < 5;i++){
        c_buffer[i] = 0;}
    int counter = 0;


  printf("Spawning 10 Prods\n");
  int alive_prod=0;
  for (i = 0; i<10; ++i) {

    disastrOS_spawn(ProdFunction,0);
    alive_prod++;
  }

  printf("Spawning 10 Cons\n");
  int alive_cons=0;
  for (i=0; i<10; ++i) {
    disastrOS_spawn(ConsFunction,0);
    alive_cons++;
  }
  int pid_counter = 0;
  while(pid_counter < 20){
    disastrOS_wait(0,NULL);
    pid_counter++;
  }


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
