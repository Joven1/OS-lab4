#include "usertraps.h"
#include "misc.h"

int recursive_stack(int stack_size)
{
	Printf("Stack Size: %d\n", stack_size);
	if(stack_size != 0)
	{
		stack_size = stack_size - 1;
		recursive_stack(stack_size);
	}
	else
	{
		return 0;
	}
}

void main (int argc, char *argv[])
{
  sem_t s_procs_completed; // Semaphore to signal the original process that we're done
  int n = 1000; // Since we are calling this function 1000 times, eventually we will cause the userstack to grow larger than the memory available

  if (argc != 2) { 
    Printf("Usage: %s <handle_to_procs_completed_semaphore>\n"); 
    Exit();
  } 

  // Convert the command-line strings into integers for use as handles
  s_procs_completed = dstrtol(argv[1], NULL, 10);

	Printf("%d Calls Made on Recursive Stack\n", recursive_stack(n));

  // Signal the semaphore to tell the original process that we're done
  if(sem_signal(s_procs_completed) != SYNC_SUCCESS) {
    Printf("hello_world (%d): Bad semaphore s_procs_completed (%d)!\n", getpid(), s_procs_completed);
    Exit();
  }

}
