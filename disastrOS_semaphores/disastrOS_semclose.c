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

  ListHead descriptor_list = running->sem_descriptors;
  SemDescriptor* semdescriptor = MySearch(&descriptor_list, semnum);

  Semaphore* semaphore = SemaphoreList_byId(&semaphores_list, semnum);
  ListHead descriptor_ptr_list = semaphore->descriptors;
  SemDescriptorPtr* semdescriptor_ptr = SemDescriptorPtrList_bySd(&descriptor_ptr_list, semdescriptor);

  if(semdescriptor){
    //devo eliminare il sem_descriptor dalla lista dei descrittori del processo (SemDescriptor) 1
    semdescriptor = (SemDescriptor*) List_detach(&descriptor_list, (ListItem*) semdescriptor);
    ret = SemDescriptor_free(semdescriptor);
    if(ret != 0) {
      running->syscall_retvalue = ERRORRET;
      return;
    }

    //devo eliminare il puntatore a descrittore dalla lista dei descrittori del semaforo (SemDescriptorPtr) 2
    semdescriptor_ptr = (SemDescriptorPtr*) List_detach(&descriptor_ptr_list, (ListItem*) semdescriptor_ptr);
    ret = SemDescriptorPtr_free (semdescriptor_ptr);
    if(ret != 0) {
      running->syscall_retvalue = ERRORRET;
      return;
    }

  }
  else {
    running->syscall_retvalue = NOTFOUND;
    return;
  }
  //devo eliminare il semaforo dalla lista dei semafori (Semaphore) 3 se non ci sono processi

  if(descriptor_ptr_list.size == 0){
    semaphore = (Semaphore*) List_detach(&semaphores_list, (ListItem*) semaphore);
    ret = Semaphore_free(semaphore);
    if(ret != 0) {
      running->syscall_retvalue = ERRORRET;
      return;
    }

  }

  running->syscall_retvalue = OK;


}
