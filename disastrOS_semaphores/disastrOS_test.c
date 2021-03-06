#include <stdio.h>
#include <unistd.h>
#include <poll.h>

#include "disastrOS.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_globals.h"

// we need this to handle the sleep state
void sleeperFunction(void* args){
  printf("Hello, I am the sleeper, and I sleep %d\n",disastrOS_getpid());
  while(1) {
    getc(stdin);
    disastrOS_printStatus();
  }
}

void childFunction(void* args){
  printf("Hello, I am the child function %d\n",disastrOS_getpid());
  printf("I will iterate a bit, before terminating\n");
  int type=0;
  int mode=0;
  int fd=disastrOS_openResource(disastrOS_getpid(),type,mode);
  printf("fd=%d\n", fd);
  printf("PID: %d, terminating\n", disastrOS_getpid());

  for (int i=0; i<(disastrOS_getpid()+1); ++i){
    printf("PID: %d, iterate %d\n", disastrOS_getpid(), i);
    disastrOS_sleep((20-disastrOS_getpid())*5);
  }
  disastrOS_exit(disastrOS_getpid()+1);
}

void semaphore_testing(void* args){

  //printf("OPEN SEM IN CHILD\n");

  int semnum1 = 10;
  int semnum2 = 30;

  //disastrOS_printStatus();
  int fd1 = disastrOS_semOpen(semnum1, 0);
  //disastrOS_printStatus();
  int fd2 = disastrOS_semOpen(semnum2, 0);


  //disastrOS_printStatus();

  //printf("Value ret of semopen: %d\n", fd);

  printf("*******************************START SEMPOST IN CHILD***********************************\n");

  disastrOS_printStatus();

  int ret;
  ret = disastrOS_semPost(fd1);

  printf("*******************************END SEMPOST IN CHILD***********************************\n");

  disastrOS_printStatus();

  printf("*******************************START SEMWAIT IN CHILD***********************************\n");

  ret = disastrOS_semWait(fd2);

  printf("*******************************END SEMWAIT IN CHILD***********************************\n");

  //printf("*******************************EXIT FROM SEMWAIT IN CHILD***********************************\n");

  printf("Value ret of semWait: %d\n", ret);

  disastrOS_printStatus();

  //disastrOS_printStatus();

  ret = disastrOS_semClose(fd1);
  //disastrOS_printStatus();
  ret = disastrOS_semClose(fd2);
  //disastrOS_printStatus();
  //disastrOS_printStatus();

  //printf("Value ret of semclose: %d\n", ret);
  //printf("I AM HERE,segmenting after this exit :( \n");
  //disastrOS_printStatus();

  disastrOS_exit(disastrOS_getpid()+1); //IMPORTANT.. if you don't want to burn your cpu

}


void initFunction(void* args) {
  //disastrOS_printStatus();
  //printf("hello, I am init and I just started\n");

  int fd1, fd2, ret;

  int semnum1 = 10;
  int semnum2 = 30;


  //testing open and close for a single process
  //printf("OPEN SEM\n");

  //fd1 = disastrOS_semOpen(semnum1, 4);
  //disastrOS_printStatus();

  //printf("Value ret of semopen: %d\n", fd1);

  //printf("OPEN SEM ANOTHER TIME\n");

  fd1 = disastrOS_semOpen(semnum1,0);
  //disastrOS_printStatus();

  printf("Value ret of semopen: %d\n", fd1);


  //printf("OPEN SEM\n");

  fd2 = disastrOS_semOpen(semnum2, 0);
  //disastrOS_printStatus();

  printf("Value ret of semopen: %d\n", fd2);

  //testing wait and post

  //Semaphore_print_count(&semaphores_list, semnum1);

  //ret = disastrOS_semPost(fd2);

  //Semaphore_print_count(&semaphores_list, semnum1);

  //Now i spawn a new process

  disastrOS_spawn(semaphore_testing, 0);

  //disastrOS_printStatus();

  int pid;
  int retval;

  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^START SEMWAIT IN FATHER^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  disastrOS_printStatus();

  ret = disastrOS_semWait(fd1);


  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^END SEMWAIT IN FATHER^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  disastrOS_printStatus();

  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^START SEMPOST IN FATHER^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  ret = disastrOS_semPost(fd2);

  printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^END SEMPOST IN FATHER^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");

  disastrOS_printStatus();

  printf("Value ret of semPost: %d\n", ret);

  pid = disastrOS_wait(0, &retval);

  printf("Pid: %d, Retval: %d\n", pid, retval);

  //child closed and now we close semaphores also for init

  //printf("CLOSE SEM\n");
  ret = disastrOS_semClose(fd1);
  //disastrOS_printStatus();

  //printf("Value ret of semclose: %d\n", ret);

  //printf("CLOSE SEM\n");
  //ret = disastrOS_semClose(fd2);
  //disastrOS_printStatus();

  //printf("Value ret of semclose: %d\n", ret);

  //printf("CLOSE SEM ANOTHER TIME\n");
  ret = disastrOS_semClose(fd2);
  //disastrOS_printStatus();

  //printf("Value ret of semclose: %d\n", ret);

  /*
  disastrOS_spawn(sleeperFunction, 0);


  printf("I feel like to spawn 10 nice threads\n");
  int alive_children=0;
  for (int i=0; i<10; ++i) {
    int type=0;
    int mode=DSOS_CREATE;
    printf("mode: %d\n", mode);
    printf("opening resource (and creating if necessary)\n");
    int fd=disastrOS_openResource(i,type,mode);
    printf("fd=%d\n", fd);
    disastrOS_spawn(childFunction, 0);
    alive_children++;
  }

  disastrOS_printStatus();
  int retval;
  int pid;
  while(alive_children>0 && (pid=disastrOS_wait(0, &retval))>=0){
    disastrOS_printStatus();
    printf("initFunction, child: %d terminated, retval:%d, alive: %d \n",
	   pid, retval, alive_children);
    --alive_children;
  }*/

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
  printf("the function pointer is: %p", childFunction);
  // spawn an init process
  printf("start\n");
  disastrOS_start(initFunction, 0, logfilename);
  return 0;
}
