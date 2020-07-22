#include "usertraps.h"
#include "misc.h"
#include "os/memory_constants.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
	int * out_of_bounds;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Unallocated Virtual Access (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

	//Still within the Virtual Address space, but less than the page allocated
	out_of_bounds = (int *) (145 * MEM_PAGESIZE);
	*out_of_bounds = 80;
  	
	Printf("Accessing.... Location: %d Value: %d\n",out_of_bounds, *out_of_bounds);

  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("Unallocated Virtual Access (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }
}

