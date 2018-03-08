#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#include "disastrOS_constants.h"


void internal_semWait(){
  // Let's do stuff!

    int fd = running->syscall_args[0];

    //let's check if the process has opened the semaphore
    SemDescriptor* semdescriptor = (SemDescriptor*) SemDescriptorList_byFd(&(running->sem_descriptors), fd);

    if(!semdescriptor) {
        //the process didn't open the semaphore..return an error
        running->syscall_retvalue = DSOS_ENOTMYSEM;
        return;
    }

    //now we check count to choose our behaviour
    Semaphore* semaphore = semdescriptor->semaphore;

    SemDescriptorPtr* semdescriptorptr = SemDescriptorPtr_alloc(semdescriptor);

    //count<=0 so the process should be put in the waiting list of the semaphore
    if (semaphore -> count <= 0){
        List_insert(&(semdescriptor->semaphore->waiting_descriptors), semaphore->waiting_descriptors.last, (ListItem*) semdescriptorptr);
    }
    //decrease the counter
    semaphore -> count-=1;

    if (semaphore -> count < 0){

        //scheduling : putting the running process in waiting and starting the next one
        running->status = Waiting;
        List_insert(&waiting_list, waiting_list.last, (ListItem*) running);
        if (ready_list.first)
            running=(PCB*) List_detach(&ready_list, ready_list.first);
        else {
            running=0;
            printf ("No process can run\n");
    }

    }

    running -> syscall_retvalue = 0;
    return;
}
