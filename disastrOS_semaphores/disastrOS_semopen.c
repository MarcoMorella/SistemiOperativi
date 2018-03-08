#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"

void internal_semOpen(){
  //let's do stuff ;)

  //we need the semnum and value
  int semnum = running->syscall_args[0];

  int value = running->syscall_args[1];

  //let's check if the semaphore isn't already opened
  ListHead semaphores = running->sem_descriptors; //semaphores open in this process
  SemDescriptor* check1 = MySearch(&semaphores, semnum); //this returns 1 if the process already has a semaphore with id = semnum

  if(check1){

      running->syscall_retvalue = check1->fd;
      return;
  }

  //semaphores_list is defined in disastrOS.c and declared in disastrOS_globals
  Semaphore* ourSem = SemaphoreList_byId(&semaphores_list, semnum); //checking if the semaphore is opened in the system
  //^this either returns a clean semaphore uninitiliazed either an already initialized semaphore,we can use it as our semaphore.

  if(!ourSem) { //if its opened we don't want to re-alloc

    ourSem = Semaphore_alloc(semnum,value);
    //there shouldn't be any instance where this returns an error,so no check here

    //we are adding the new semaphore to the global semaphores list of disastrOS (defined in disastros.c)
    List_insert(&semaphores_list,semaphores_list.last,(ListItem*) ourSem); //first argument is the first element,second is the last element,sem is the element we want to insert,casted to ListItem as required
  }
  //let's create a descriptor so we can add it to the PCB of this process


  SemDescriptor* dsc = SemDescriptor_alloc(running->last_sem_fd,ourSem,running);   //returns 0 if there is any error
  if(!dsc) {
      running->syscall_retvalue = DSOS_ECREATEFD;
      return;
  }

  // PCB of our running process has int last_sem_fd; so we add 1 to give the next semaphore a different fd
  running->last_sem_fd+=1;

  //now the pointer
  SemDescriptorPtr * ptr = SemDescriptorPtr_alloc(dsc);
  dsc->ptr = ptr;

  //adding the dsc to the list of the process
  List_insert(&running-> sem_descriptors,running->sem_descriptors.last,(ListItem*) dsc);

  //adding the ptr to the list of the semaphore
  List_insert(&ourSem -> descriptors, ourSem-> descriptors.last, (ListItem*) ptr);

  running -> syscall_retvalue = dsc -> fd ;
 return;
}
