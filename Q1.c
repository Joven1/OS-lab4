//I added this file to make it easier to see on vim
//I still have no idea what I'm doing

Description:
Implement dynamic one-level paging in DLXOS. All your code should go in the one-level/ directory. 

You will need to write code in process.c, memory.c, process.h, memory.h, and memory_constants.h. 

Your implementation must abide by the following specifications:

TODO:
Use a virtual memory size of 1024KB (note that 1024KB is not exactly 1,024,000 bytes).

Use a maximum physical memory size of 2MB (again, 2MB is not exactly 2,000,000 bytes).

You can only hard-code values for MEM_L1FIELD_FIRST_BITNUM, MAX_VIRTUAL_ADDRESS, MEM_MAX_SIZE, MEM_PTE_READONLY, MEM_PTE_DIRTY, and MEM_PTE_VALID. All other memory-related constants must be computed from those 6 in code. MEM_MAX_SIZE is the maximum physical memory size.

When creating a process, initially allocate 4 pages for code and global data, 1 page for the user stack, and one page for the system stack. Assume that the system stack will never grow larger than 1 page.

You will need to implement a page fault handler which allocates a new page if the user stack causes a page fault, and kills a process for any other page faults.

DO NOT EXIT THE SIMULATOR IF SOMETHING GOES WRONG. You should kill the process that caused the problem with ProcessKill(), and then continue running the OS.
