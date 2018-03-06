#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"

#define ERRORRET -2 //memory error by free functions
#define NOTFOUND -1 //semaphore with semnum passed it's not used by this process or doesn't exist
#define OK 0

void internal_semClose(){

  int semnum = running->syscall_args[0];
  int ret;

  Semaphore* semaphore = SemaphoreList_byId(&semaphores_list, semnum);
  if(!semaphore){ //if semaphore == NULL it means that semaphore doesn't exist
    running->syscall_retvalue = NOTFOUND;
    return;
  }

  SemDescriptor* semdescriptor = MySearch(&(running->sem_descriptors), semnum);
  if(!semdescriptor){ //check if there's the semaphore in this process
    running->syscall_retvalue = NOTFOUND; //there aren't semaphore with id = semnum in this process
    return;
  }

  SemDescriptorPtr* semdescriptor_ptr = SemDescriptorPtrList_bySd(&(semaphore->descriptors), semdescriptor);
  if(!semdescriptor_ptr){ //this control it's not necessary (because if semdescriptor != NULL => semdescriptor_ptr != NULL)
    running->syscall_retvalue = NOTFOUND;
    return;
  }

  //delete semdescriptor from descriptor's list of the process that call this function
  semdescriptor = (SemDescriptor*) List_detach(&(running->sem_descriptors), (ListItem*) semdescriptor);
  ret = SemDescriptor_free(semdescriptor);
  if(ret != 0) {
    running->syscall_retvalue = ERRORRET;
    return;
  }

  //delete semdescriptor_ptr from semDescriptorPtr's list of the semaphore with id = semnum
  semdescriptor_ptr = (SemDescriptorPtr*) List_detach(&(semaphore->descriptors), (ListItem*) semdescriptor_ptr);
  ret = SemDescriptorPtr_free (semdescriptor_ptr);
  if(ret != 0) {
    running->syscall_retvalue = ERRORRET;
    return;
  }

  //now if semaphore doesn't have any descriptorPtr it means that it should be unlinked
  if((semaphore->descriptors).size == 0){
    semaphore = (Semaphore*) List_detach(&semaphores_list, (ListItem*) semaphore);
    ret = Semaphore_free(semaphore);
    if(ret != 0) {
      running->syscall_retvalue = ERRORRET;
      return;
    }

  }

  running->syscall_retvalue = OK;

}
