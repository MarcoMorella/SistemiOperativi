#include <assert.h>
#include <unistd.h>
#include <stdio.h>
#include "disastrOS.h"
#include "disastrOS_syscalls.h"
#include "disastrOS_semaphore.h"
#include "disastrOS_semdescriptor.h"
#include "disastrOS_globals.h"
#define ERRORNOTMYSEM -40

void internal_semWait(){
  // do stuff :)

    int semnum = running->syscall_args[0];

    //let's check if the process has opened the semaphore

    SemDescriptor* searching_sem = MySearch(&running->sem_descriptors,semnum);

    if(!searching_sem) {
        running->syscall_retvalue = ERRORNOTMYSEM;
        return;
}
    return;
}
