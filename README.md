This is my implementation of lab 4 for ECE 469. 
### Building and Running:
In order to build/run each part of the lab, go inside each individual directory (one-level/fork/heap-mgmt) and execute the ./run.sh command. This will first compile the os code, then the user programs to test the os code. 

### Part 1: Page Tables
The memory constants that were adjusted are under process.h/memory_constants.h/memory.h. Inside the freemap, I decided to set '1' as "inuse" and '0' as "not inuse" as seen in the MemoryFreePageFunction. I mention to ease up any confusion. To test my os code, I split question 2 into 6 different directories to make viewing easier. This is seen under apps/example. 

Issues: When running part 6, I attempted to run 30 simultaneous processes. I'm not that confident this is reliable since it is a known bug that DLXOS appears t dump core after more than 10 processes are running. See below:
http://www.mscs.mu.edu/~brylow/ece469/Spring2004/Labs/dlxos_bugs.html


### Part 2: Fork
Code was first copied directly from the page tables. Only adjustment that was made was the handler when a read only page access violation occurs and ProcessRealFork. Inside the testing program, I had one process fork 8 different processes that would then print out the parent and child process's page tables.

### Part 3: Buddy Memory Allocation
Code was first copied directly from the page tables. Adjustments were made in just process.c and memory.c. This code implements the malloc and free using the buddy memory allocation system. You can test this under apps/example/makeprocs/makeprocs.c. In this case, I have 4 different "programs" that have different memory requirements.

Issues: When merging and splitting nodes, I originally decided to just use for loops. However, my code ended up seg faulting multiple times. I find it looks more elegant and cleaner if you use recursion when splitting and merging memory segments. Since growth in orders of memory segments are exponential and not linear, I see no problem in the recursive stack growing too large. 
