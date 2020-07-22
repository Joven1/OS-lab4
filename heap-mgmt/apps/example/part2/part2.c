#include "usertraps.h"
#include "misc.h"

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int* memory_location; // Somewhere in the middle of 1024KB memory space
	int * out_of_bounds;

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

	//Access Memory Beyond Address Space
	Printf("Accessing Memory at the MEM_MAX_VIRTUAL_ADDRESS + 1\n");
	out_of_bounds = 0x100000 + 1;
	*out_of_bounds = 80;
	
	Printf("Accessing.... Location: %d Value: %d\n",out_of_bounds, *out_of_bounds); 

	if(sem_signal(s_procs_completed) != SYNC_SUCCESS)
	{
		Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
		Exit();
	}
	 
}
