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

    int semnum = running->syscall_args[0];

    //let's check if the process has opened the semaphore
    SemDescriptor* searching_sem = MySearch(&running->sem_descriptors,semnum);

    if(!searching_sem) {
        //the process didn't open the semaphore..return an error
        running->syscall_retvalue = DSOS_ENOTMYSEM;
        return;
    }

    //now we check count to choose our behaviour
    Semaphore* semaphore = searching_sem ->semaphore;

    //count<=0 so the process should be put in the waiting list of the semaphore
    if (semaphore -> count <= 0){
            List_insert(&semaphore->waiting_descriptors,semaphore->waiting_descriptors.last, (ListItem*) running);
    }
    //decrease the counter
    semaphore -> count-=1;

    if (semaphore -> count < 0){

        //let's remove the process from the running list and place him in the waiting list of the OS
        List_detach(&ready_list,(ListItem*) running);
        List_insert(&waiting_list,waiting_list.last,(ListItem*) running);

        //and change his status to waiting
        running -> status = Waiting;
    }


    running -> syscall_retvalue = 0;
    return;
}
