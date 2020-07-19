#include "usertraps.h"
#include "misc.h"

//Access Memory Constants
#include "os/memory_constants.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done

	int * address; //Address Accessing

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 	

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  // Now print a message to show that everything worked
  Printf("hello_world (%d): Hello world!\n", getpid());

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }
	//Test 2

	Printf("Test 2: Out of Bounds\n");
	address = (MEM_MAX_VIRTUAL_ADDRESS + 1);
	Printf("Test 2 (%d) : Accessing Memory Location: %d \n", getpid(), address);
	Printf("Value is: %d\n", *address); //Access Memory Beyond Virtual Address
 	
  Printf("hello_world (%d): Done!\n", getpid());
}
