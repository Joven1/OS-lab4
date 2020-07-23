#ifndef	_memory_constants_h_
#define	_memory_constants_h_

//------------------------------------------------
// #define's that you are given:
//------------------------------------------------

// We can read this address in I/O space to figure out how much memory
// is available on the system.
#define	DLX_MEMSIZE_ADDRESS	0xffff0000

// Return values for success and failure of functions
#define MEM_SUCCESS 1
#define MEM_FAIL -1

//--------------------------------------------------------
// Put your constant definitions related to memory here.
// Be sure to prepend any constant names with "MEM_" so 
// that the grader knows they are defined in this file.

//Bit Position of LSB in Virtual Address
#define MEM_L1FIELD_FIRST_BITNUM 12

//Max Virtual Address (Size is 1024 KB -> 1048576 Bytes)
#define MEM_MAX_VIRTUAL_ADDRESS 1048575

//Mem Max Size (Size is 2MB -> 2097152 Bytes)
#define MEM_MAX_SIZE 2097152

//Copied from Lab 4
#define MEM_PTE_READONLY 0x4
#define MEM_PTE_DIRTY 0x2 
#define MEM_PTE_VALID 0x1


//Computable Results

#define MEM_PAGESIZE (0x1 << MEM_L1FIELD_FIRST_BITNUM)
#define MEM_L1_PAGETABLE_SIZE ((MEM_MAX_VIRTUAL_ADDRESS + 1) >> MEM_L1FIELD_FIRST_BITNUM) 
#define MEM_ADDRESS_OFFSET_MASK (MEM_PAGESIZE - 1)
#define MEM_MASK_PTE_TO_PAGE_ADDRESS (~(MEM_PTE_READONLY | MEM_PTE_DIRTY | MEM_PTE_VALID)) 

#define HEAP_FIRST_BITNUM 7
#define MAX_HEAP_BLOCKS 128 //Max number of heap blocks is 4096/32 = 128
#define MAX_HEAP_ORDER 7 //Max order is log2(128 Blocks) = 7
#define HEAP_BLOCK_USE_MASK 0x80000000 //Mask to see if block is inuse. 0 = Available, 1 = Not Available
#define HEAP_ORDER_MASK 0x7FFFFFFF //Mask to see the order of the block

#define MEM_HEAP_MIN_BLOCKSIZE 32


//--------------------------------------------------------


#endif	// _memory_constants_h_
