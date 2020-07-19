#include "usertraps.h"
#include "misc.h"

#define HELLO_WORLD "hello_world.dlx.obj"
#define PART1   "part1.dlx.obj"
#define PART2   "part2.dlx.obj"
#define PART3   "part3.dlx.obj"
#define PART4   "part4.dlx.obj"
#define PART5   "part5.dlx.obj"
#define PART6   "part6.dlx.obj"


void main (int argc, char *argv[])
{
    int i;                               // Loop index variable
    sem_t s_procs_completed;             // Semaphore used to wait until all spawned processes have completed
    char s_procs_completed_str[10];      // Used as command-line argument to pass page_mapped handle to new processes
    int program;                        // used to figure out which program to run

    if (argc != 2) {
        Printf("Usage: %s <which program to run (1-6) or all of them (0) \n", argv[0]);
        Exit();
    }

    // Convert string from ascii command line argument to integer number
    program = dstrtol(argv[1], NULL, 10); // the "10" means base 10

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

	//Student
	switch(program)
	{
		case 1:
			Printf("\n========================================\n");
			Printf("PART 1 START");
			Printf("\n========================================\n\n");
			
			process_create(PART1, s_procs_completed_str,NULL);	
			if(sem_wait(s_procs_completed) != SYNC_SUCCESS)
			{
				Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
				Exit();
			}			
			
			Printf("\n\n========================================\n");
			Printf("PART 1 END\n");
			Printf("\n========================================\n");
			break;

		case 2:
			Printf("\n========================================\n");
			Printf("PART 2 START");
			Printf("\n========================================\n\n");
	
			process_create(PART2, s_procs_completed_str,NULL);
			if(sem_wait(s_procs_completed) != SYNC_SUCCESS)
			{
				Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
				Exit();
			}
			
			Printf("\n\n========================================\n");
			Printf("PART 2 END\n");
			Printf("\n========================================\n");		
			break;

		case 3:				
			Printf("\n========================================\n");
			Printf("PART 3 START");
			Printf("\n========================================\n\n");

			process_create(PART3, s_procs_completed_str, NULL);
			
			if(sem_wait(s_procs_completed) != SYNC_SUCCESS)
			{
				Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
				Exit();
			}
			
			Printf("\n\n========================================\n");
			Printf("PART 3 END\n");
			Printf("\n========================================\n");
			break;
			
		case 4:			
			Printf("\n========================================\n");
			Printf("PART 4 START");
			Printf("\n========================================\n\n");

			process_create(PART4, s_procs_completed_str,NULL); 
	
			if(sem_wait(s_procs_completed) != SYNC_SUCCESS)
			{
				Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
				Exit();
			}
		
			Printf("\n\n========================================\n");
			Printf("PART 4 END\n");
			Printf("\n========================================\n");
			break;	
	
		case 5:			
			Printf("\n========================================\n");
			Printf("PART 5 START");
			Printf("\n========================================\n\n");

			//Call Hello World (Part 1) 100 times
			for(i = 0; i < 100; i++)
			{
				process_create(PART1, s_procs_completed_str,NULL); 
				if(sem_wait(s_procs_completed) != SYNC_SUCCESS)
				{
					Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
					Exit();
				}
			}
	
			
			Printf("\n\n========================================\n");
			Printf("PART 5 END\n");
			Printf("\n========================================\n");
			break;

		case 6:			
			Printf("\n========================================\n");
			Printf("PART 6 START");
			Printf("\n========================================\n\n");

			//Spawn 30 Processes
			for(i = 0; i < 30; i++)
			{
				process_create(PART6, s_procs_completed_str,NULL); 
			}
			if(sem_wait(s_procs_completed) != SYNC_SUCCESS)
			{
				Printf("Bad semaphore s_procs_completed (%d) in %s\n", s_procs_completed, argv[0]);
				Exit();
			}		
			Printf("\n\n========================================\n");
			Printf("PART 6 END\n");
			Printf("\n========================================\n");
			break;
	}
    Printf("makeprocs (%d): All other processes completed, exiting main process.\n", getpid());

}
