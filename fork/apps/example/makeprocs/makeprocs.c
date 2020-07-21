#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"


void main (int argc, char *argv[])
{
    int i;                               // Loop index variable
    sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
    char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
    int forks;                        // used to figure out which program to run
    int child_pid; //child forking


    if (argc != 2) {
        Printf("Usage: %s <which program to run (1-6) or all of them (0) \n", argv[0]);
        Exit();
    }

    // Convert string from ascii command line argument to integer number
    forks = dstrtol(argv[1], NULL, 10); // the "10" means base 10

    // Create semaphore to not exit this process until all other processes 
    // have signalled that they are complete.
    if ((s_procs_completed = sem_create(0)) == SYNC_FAIL) {
        Printf("makeprocs (%d): Bad sem_create\n", getpid());
        Exit();
    }

    // Setup the command-line arguments for the new processes.  We're going to
    // pass the handles to the semaphore as strings
    // on the command line, so we must first convert them from ints to strings.
    ditoa(s_procs_completed, s_procs_completed_str);

	for(i = 0; i < forks; i++)
	{
		Printf("Process: (%d) Fork Number: (%d)\n", getpid(), i);
		child_pid = fork();
		if(child_pid != 0)
		{
			Printf("This is a Parent Process: (%d)\n", getpid());
			Printf("The Child (%d) ID: %d\n", i, child_pid);
		}	
		else
		{
			Printf("This is a Child (%d): (%d)\n", i, getpid());	
		}
	}


    Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
