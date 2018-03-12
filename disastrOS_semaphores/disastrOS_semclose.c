#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"


void internal_semClose(){

  int fd = running->syscall_args[0];
  int ret;

  //get semdescriptor from sem_descriptors of current process
  SemDescriptor*  semdescriptor = SemDescriptorList_byFd(&running->sem_descriptors, fd);
  if(!semdescriptor){ //check if there's the semaphore in this process
    running->syscall_retvalue = DSOS_ENOTMYSEM; //there aren't semaphore with id = semnum in this process
    return;
  }

  //get semaphore from semdescriptor
  Semaphore* semaphore = semdescriptor->semaphore; //if semdescriptor exists => semaphore exists

  SemDescriptorPtr* semdescriptor_ptr = semdescriptor->ptr;
  //semdescriptor_ptr's control it's not necessary (because if semdescriptor != NULL => semdescriptor_ptr != NULL)

  SemDescriptorPtr* semdescriptor_wait_ptr = semdescriptor->wait_ptr;

  //delete semdescriptor from descriptor's list of the process that call this function
  semdescriptor = (SemDescriptor*) List_detach(&(running->sem_descriptors), (ListItem*) semdescriptor);
  ret = SemDescriptor_free(semdescriptor);
  if(ret) {
    running->syscall_retvalue = ret;
    return;
  }

  //delete semdescriptor_ptr from semDescriptorPtr's list of the semaphore with id = semnum
  semdescriptor_ptr = (SemDescriptorPtr*) List_detach(&(semaphore->descriptors), (ListItem*) semdescriptor_ptr);
  ret = SemDescriptorPtr_free (semdescriptor_ptr);
  if(ret) {
    running->syscall_retvalue = ret;
    return;
  }

  ret = SemDescriptorPtr_free (semdescriptor_wait_ptr); //free semDescriptorPtr used to put it in waiting_descriptors
  if(ret) {
    running->syscall_retvalue = ret;
    return;
  }

  //now if semaphore doesn't have any descriptorPtr it means that it should be unlinked
  if((semaphore->descriptors).size == 0){
    semaphore = (Semaphore*) List_detach(&semaphores_list, (ListItem*) semaphore);
    ret = Semaphore_free(semaphore);
    if(ret != 0) {
      running->syscall_retvalue = ret;
      return;
    }

  }

  running->syscall_retvalue = 0;

}
