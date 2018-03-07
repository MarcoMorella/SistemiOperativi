#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"


void internal_semPost(){

  int fd = running->syscall_args[0];

  SemDescriptor* semdescriptor = (SemDescriptor*) SemDescriptorList_byFd(&(running->sem_descriptors), fd);
  if(!semdescriptor){ //semdescriptor doesn't exist
    running->syscall_retvalue = DSOS_ERESOURCENOFD;
    return;
  }

  Semaphore* semaphore = semdescriptor->semaphore; //semaphore has to exist if semdescriptor exist

  if(semaphore->count < 0){

    //take head descriptor in semaphore's waiting_descriptors
    SemDescriptorPtr* head_wait_descriptor = (SemDescriptorPtr*) List_detach(&(semaphore->waiting_descriptors), (ListItem*) (semaphore->waiting_descriptors).first);
    //head_wait_descriptor can't be NULL if count < 0 because has to exist one process that wait

    //drop pcb of process with head_wait_descriptor from waiting_list and put it in ready_list
    PCB* pcb_head = head_wait_descriptor->descriptor->pcb;

    List_detach(&waiting_list, (ListItem*) pcb_head);
    List_insert(&ready_list, (ListItem*) ready_list.last, (ListItem*) pcb_head);

    //mark pcb in ready status
    pcb_head->status = Ready;

  }

  //increment count
  (semaphore->count)++;

  running->syscall_retvalue = 0;

}
