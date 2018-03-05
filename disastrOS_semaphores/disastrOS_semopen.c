#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"


#define ERRORINFD -64
void internal_semOpen(){
  //let's do stuff ;)

  //we need the semnum and value
  int semnum = running->syscall_args[0];

  int value = running->syscall_args[1];

  //let's check if the semaphore isn't already opened
  ListHead semaphores = running->sem_descriptors; //semaphores open in this process
  Semaphore* ourSem = SemaphoreList_byId(&semaphores, semnum); //this should return a sem with the same id as semnum

  if(ourSem) {
        //so the semaphore is already open so we return semnum
        running->syscall_retvalue = semnum;
        return;
  }

  ourSem = Semaphore_alloc(semnum,value);
  //there shouldn't be any instance where this returns an error,so no check here

  //we are adding the new semaphore to the global semaphores list of our OS (defined in disastros.c)
  List_insert(&semaphores_list,semaphores_list.last,(ListItem*) ourSem); //first argument is the first element,second is the last element,sem is the element we want to insert,casted to ListItem as required
  //semaphores_list is the global list of disastros's semaphores. It is declared and initialized in disastros.c but it doesn't seem to work here.. i'll have to check this later.

  //let's create a descriptor so we can add it to the PCB of this process

  SemDescriptor* dsc = SemDescriptor_alloc(semnum,ourSem,running);   //returns 0 if there is any error
  if(!dsc) {
      running->syscall_retvalue = ERRORINFD;
      return;
  }

  // PCB of our running process has int last_sem_fd; so we add 1 to give the next semaphore a different fd
  //running->last_sem_fd+=1;

  //now the pointer
  SemDescriptorPtr * ptr = SemDescriptorPtr_alloc(dsc);
  dsc->ptr = ptr;

  //adding the ptr to the list of the process
  List_insert(&running-> sem_descriptors,running->sem_descriptors.last,(ListItem*) dsc);

  //adding the ptr to the list of the semaphore
  List_insert(&ourSem -> descriptors, ourSem-> descriptors.last, (ListItem*) ptr);

  running -> syscall_retvalue = dsc -> fd ; // dsc -> fd is semnum
 return;
}
