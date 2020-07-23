//
//	memory.c
//
//	Routines for dealing with memory management.

//static char rcsid[] = "$Id: memory.c,v 1.1 2000/09/20 01:50:19 elm Exp elm $";

#include "ostraps.h"
#include "dlxos.h"
#include "process.h"
#include "memory.h"
#include "queue.h"

// num_pages = size_of_memory / size_of_one_page
static uint32 freemap_size = (MEM_MAX_SIZE/MEM_PAGESIZE)/32;
static uint32 freemap[(MEM_MAX_SIZE/MEM_PAGESIZE)/32]; //Max amount of memory divided by 32 since each bit in the uint32 says 1 = free 0 = not free
static uint32 pagestart;
static int nfreepages;
static int freemapmax;

//----------------------------------------------------------------------
//
//	This silliness is required because the compiler believes that
//	it can invert a number by subtracting it from zero and subtracting
//	an additional 1.  This works unless you try to negate 0x80000000,
//	which causes an overflow when subtracted from 0.  Simply
//	trying to do an XOR with 0xffffffff results in the same code
//	being emitted.
//
//----------------------------------------------------------------------
static int negativeone = 0xFFFFFFFF;
static inline uint32 invert (uint32 n) {
  return (n ^ negativeone);
}

//----------------------------------------------------------------------
//
//	MemoryGetSize
//
//	Return the total size of memory in the simulator.  This is
//	available by reading a special location.
//
//----------------------------------------------------------------------
int MemoryGetSize() {
  return (*((int *)DLX_MEMSIZE_ADDRESS));
}


//----------------------------------------------------------------------
//
//	MemoryModuleInit
//
//	Initialize the memory module of the operating system.
//      Basically just need to setup the freemap for pages, and mark
//      the ones in use by the operating system as "VALID", and mark
//      all the rest as not in use.
//
//----------------------------------------------------------------------
void MemoryModuleInit() 
{
	
	int i,j;
	uint32 mask;
	uint32 os_pages = (lastosaddress >> MEM_L1FIELD_FIRST_BITNUM); //Calculate number of pages occupied by operating system
	if((os_pages % MEM_PAGESIZE) != 0) //Incase of rounding error, round up
	{
		os_pages++;
	}


	for(i = 0; i < freemap_size; i++) //Loop through number of indices in the free map
	{
		freemap[i] = 0; //Set garbage value as zero
		mask = 0x1; //Mask for setting the bits
		for(j = 0; j < 32; j++)	
		{
			//For each amount of OS_PAGES used, mark as a 0 (not in use) and mark the rest as 1 (in use)
			if(os_pages == 0)
			{
				freemap[i] = freemap[i] | mask;
				mask = mask << 1;
			}
			else 
			{
				os_pages = os_pages - 1; //Keep on subtracting pages occupied by OS 
				mask = mask << 1;
			}
		}
		
	}
}


//----------------------------------------------------------------------
//
// MemoryTranslateUserToSystem
//
//	Translate a user address (in the process referenced by pcb)
//	into an OS (physical) address.  Return the physical address.
//
//----------------------------------------------------------------------
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr) 
{
	
	uint32 page_offset; //Lower Part of Address
	uint32 page_number; //Upper Part of Address
	uint32 pte; //Page Table Entry (Value stored in page Table)
	uint32 physical_address; //OS (physical) Address

	if( addr > MEM_MAX_VIRTUAL_ADDRESS)
	{
		ProcessKill();
	}
	page_number = addr >> MEM_L1FIELD_FIRST_BITNUM; //Extract Page_ Number by right shifting the virtual address
	page_offset = addr & 0xFFF; //Last 12 Bits is Offset
	
	pte = pcb->pagetable[page_number]; //obtain entry from page table 

	
	if((pte & MEM_PTE_VALID) == 0) //Entry is not a valid physical page, throw page fault exception
	{
		return MemoryPageFaultHandler(pcb);
	}
	
	physical_address = pte & MEM_MASK_PTE_TO_PAGE_ADDRESS; //Convert from PTE to a page address
	physical_address = physical_address | page_offset; //Get the actual physical address with the page offset	

	return physical_address; //return address
}


//----------------------------------------------------------------------
//
//	MemoryMoveBetweenSpaces
//
//	Copy data between user and system spaces.  This is done page by
//	page by:
//	* Translating the user address into system space.
//	* Copying all of the data in that page
//	* Repeating until all of the data is copied.
//	A positive direction means the copy goes from system to user
//	space; negative direction means the copy goes from user to system
//	space.
//
//	This routine returns the number of bytes copied.  Note that this
//	may be less than the number requested if there were unmapped pages
//	in the user range.  If this happens, the copy stops at the
//	first unmapped address.
//
//----------------------------------------------------------------------
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir) {
  unsigned char *curUser;         // Holds current physical address representing user-space virtual address
  int		bytesCopied = 0;  // Running counter
  int		bytesToCopy;      // Used to compute number of bytes left in page to be copied

  while (n > 0) {
    // Translate current user page to system address.  If this fails, return
    // the number of bytes copied so far.
    curUser = (unsigned char *)MemoryTranslateUserToSystem (pcb, (uint32)user);

    // If we could not translate address, exit now
    if (curUser == (unsigned char *)0) break;

    // Calculate the number of bytes to copy this time.  If we have more bytes
    // to copy than there are left in the current page, we'll have to just copy to the
    // end of the page and then go through the loop again with the next page.
    // In other words, "bytesToCopy" is the minimum of the bytes left on this page 
    // and the total number of bytes left to copy ("n").

    // First, compute number of bytes left in this page.  This is just
    // the total size of a page minus the current offset part of the physical
    // address.  MEM_PAGESIZE should be the size (in bytes) of 1 page of memory.
    // MEM_ADDRESS_OFFSET_MASK should be the bit mask required to get just the
    // "offset" portion of an address.
    bytesToCopy = MEM_PAGESIZE - ((uint32)curUser & MEM_ADDRESS_OFFSET_MASK);
    
    // Now find minimum of bytes in this page vs. total bytes left to copy
    if (bytesToCopy > n) {
      bytesToCopy = n;
    }

    // Perform the copy.
    if (dir >= 0) {
      bcopy (system, curUser, bytesToCopy);
    } else {
      bcopy (curUser, system, bytesToCopy);
    }

    // Keep track of bytes copied and adjust addresses appropriately.
    n -= bytesToCopy;           // Total number of bytes left to copy
    bytesCopied += bytesToCopy; // Total number of bytes copied thus far
    system += bytesToCopy;      // Current address in system space to copy next bytes from/into
    user += bytesToCopy;        // Current virtual address in user space to copy next bytes from/into
  }
  return (bytesCopied);
}

//----------------------------------------------------------------------
//
//	These two routines copy data between user and system spaces.
//	They call a common routine to do the copying; the only difference
//	between the calls is the actual call to do the copying.  Everything
//	else is identical.
//
//----------------------------------------------------------------------
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, from, to, n, 1));
}

int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from,unsigned char *to, int n) {
  return (MemoryMoveBetweenSpaces (pcb, to, from, n, -1));
}

//---------------------------------------------------------------------
// MemoryPageFaultHandler is called in traps.c whenever a page fault 
// (better known as a "seg fault" occurs.  If the address that was
// being accessed is on the stack, we need to allocate a new page 
// for the stack.  If it is not on the stack, then this is a legitimate
// seg fault and we should kill the process.  Returns MEM_SUCCESS
// on success, and kills the current process on failure.  Note that
// fault_address is the beginning of the page of the virtual address that 
// caused the page fault, i.e. it is the vaddr with the offset zero-ed
// out.
//
// Note: The existing code is incomplete and only for reference. 
// Feel free to edit.
//---------------------------------------------------------------------
int MemoryPageFaultHandler(PCB *pcb) 
{
	//Obtain the Page Fault Address and Userstack Address by 
	uint32 page_fault_addr = pcb->currentSavedFrame[PROCESS_STACK_FAULT];
	uint32 userstack_addr = pcb->currentSavedFrame[PROCESS_STACK_USER_STACKPOINTER] & MEM_MASK_PTE_TO_PAGE_ADDRESS;
	
	//New Page incase allocation is needed
	uint32 new_page;
	uint32 page_number = page_fault_addr >> MEM_L1FIELD_FIRST_BITNUM;
	
	if(page_fault_addr >= userstack_addr) //If the fault address is greater than or equal to stack pointer
	{
		//Allocate a New Page for process
		new_page = MemoryAllocPage();
		if(new_page == MEM_FAIL)
		{
			printf("ERROR: Memory cannot be allocated\n");
			exitsim();
		}
		pcb->pagetable[page_number] = MemorySetupPte(new_page);
		
		return MEM_SUCCESS;
	}
	else
	{
		printf("Segmentation fault (core dumped)\n");
		ProcessKill();
  		return MEM_FAIL;
	}
		
}


//---------------------------------------------------------------------
// You may need to implement the following functions and access them from process.c
// Feel free to edit/remove them
//---------------------------------------------------------------------

int MemoryAllocPage(void) 
{
	//Loop Variables
	int i, j;
	uint32 mask;
	int return_value;
	uint32 check_val_and;

	for(i = 0; i < freemap_size; i++) //Loop through the freemap
	{
		mask = 0x1;
		if(freemap[i] != 0) //Found a non zero entry available
		{
			for(j = 0; j < 32; j++)
			{
				check_val_and = freemap[i] & mask;
				if(check_val_and > 0)
				{
					freemap[i] = freemap[i] ^ mask;
					return_value = (i*32) + j; //Page is number of entries looped + jth available entry
					return return_value;
				}
				else 
				{
					//Check next bits then move mask
					mask = mask << 1;
				}
			}
		}
	}
	 
	return MEM_FAIL; //No Pages are available
}


uint32 MemorySetupPte (uint32 page) 
{

	uint32 pte;
	pte = page << MEM_L1FIELD_FIRST_BITNUM; //Left shift the page number 
	pte = pte | MEM_PTE_VALID; //Set unit as valid  
	return pte;
}


//Sets Free Map entry as 1 as in "not in use"
void MemoryFreePage(uint32 page) 
{
	uint32 freemap_index;
	uint32 index_bit_position;
	uint32 mask;
	
	page = page & MEM_MASK_PTE_TO_PAGE_ADDRESS; //Mask status bits to get page number
	page = page >> MEM_L1FIELD_FIRST_BITNUM; //Get page
	
	freemap_index = page/32;
	index_bit_position = page % 32;
	
	mask = 0x1 << index_bit_position; //From the index bit position, shift the 1 to set
	
	freemap[freemap_index] = freemap[freemap_index] ^ mask; //Set the bit 
	
	nfreepages++; //number of free pages increases
	return;
	
}


//Question 5

int mfree(PCB* pcb, void* ptr) 
{
	int freeLocation = ((int) ptr & MEM_ADDRESS_OFFSET_MASK) >> HEAP_FIRST_BITNUM;
	int freeOrder = pcb->heap[freeLocation] & HEAP_ORDER_MASK;
	int parentStart = freeLocation;
	int parentEnd = freeLocation + (1 << freeOrder);
	int order = freeOrder;
	int i;

	while(1)
	{
		if(order == freeOrder)
		{
			printf("Freed the block: order = %d, addr = %d, size = %d\n", order, parentStart << HEAP_FIRST_BITNUM, (1 << order) << HEAP_FIRST_BITNUM);
		}
		else
		{
			printf("Coalesced buddy nodes (order = %d, addr = %d, size = %d) & (order = %d, addr = %d, size = %d)\n",
							order - 1, parentStart << HEAP_FIRST_BITNUM, (1 << (order - 1)) << HEAP_FIRST_BITNUM, 
							order - 1, (parentStart + (1 << (order - 1))) << HEAP_FIRST_BITNUM, (1 << (order - 1)) <<HEAP_FIRST_BITNUM);
			printf("into the parent node (order = %d, addr = %d, size = %d)\n",
							order, parentStart << HEAP_FIRST_BITNUM, (1 << order) << HEAP_FIRST_BITNUM);
		}

		for(i = parentStart; i < parentEnd; ++i)
		{
			pcb->heap[i] = HEAP_ORDER_MASK & order;
		}

		parentStart = parentStart >> (order + 1) >> (order + 1);
		parentEnd = parentStart + (1 << (order + 1));


		if((pcb->heap[parentStart] & HEAP_BLOCK_USE_MASK) != 0)
		{
			break;
		}
		else if((pcb->heap[parentEnd - 1] & HEAP_BLOCK_USE_MASK) != 0)
		{
			break;
		}
		else if((pcb->heap[parentStart] & HEAP_ORDER_MASK) != order)
		{
			break;
		}
		else if((pcb->heap[parentEnd - 1] & HEAP_ORDER_MASK) != order)
		{
			break;
		}	
		order++;
		if(order > MAX_HEAP_ORDER)
		{
			break;
		}
	}
	return 0;
}


void HeapInit(PCB * pcb)
{
	int i;
	for(i = 0; i < MAX_HEAP_BLOCKS; i++)
	{
		pcb->heap[i] = MAX_HEAP_ORDER;
	}
}

void printBinary(uint32 n)
{
	if(n)
	{
		printBinary(n >> 1);
		printf("%d", n & 1);
	}
}

void printHeap(PCB * pcb)
{	
	int i;
	for(i = 0; i < MAX_HEAP_BLOCKS; i++)
	{
		printf("Index: %d, Value: %x ,IN_USE: %x \n", i, pcb->heap[i] & HEAP_ORDER_MASK, (pcb->heap[i] & HEAP_BLOCK_USE_MASK) >> 31);
	}
	printf("\n");	
}

void * malloc(PCB * pcb, int memsize)
{
	int i; //Loop Control Variable
	int order = 0;
	int mem_block = 32; //Our block of memory first starts off as order 0
	int minOrder = MAX_HEAP_ORDER + 1, minOrderLocation = 0, upperBound = 0;
	int test1;
	int test2;
	int test3;

	//Return null if memsize is less than zero or greater than the heap
	if(memsize <= 0)
	{
		return NULL;
	}
	else if(memsize > MEM_PAGESIZE)
	{
		return NULL;
	}

	//Make sure that the memsize is 4 byte aligned, if not, round up
	if( (memsize % 4) != 0)
	{
		memsize += (4 - (memsize % 4));
	}
	
	//Create a Block Size that can fit the requested memory	
	while(memsize > mem_block)
	{
		mem_block = mem_block * 2; //Increase the mem_block size until the memsize is less than
		order++; //Increase the order as we double the block size	
	}

	
	//Find Block that is large enough to contain the requested memory (establish a lower bound)
	i = 0;
	while(i < MAX_HEAP_BLOCKS)
	{
		//3 Tests to pass
		test1 = ((pcb->heap[i] & HEAP_BLOCK_USE_MASK) == 0); //If the entry is not in use
		test2 = (pcb->heap[i] & HEAP_ORDER_MASK) >= order; //If the entry is greater than or equal to the order we requested
		test3 = (pcb->heap[i] & HEAP_ORDER_MASK) < minOrder; //If the entry is less than the minimum
		
		if( (test1 & test2) & test3)
		{
			minOrder = (pcb->heap[i] & HEAP_ORDER_MASK);
			minOrderLocation = i;
		}		
	

		i = i + (1 << (pcb->heap[i] & HEAP_ORDER_MASK));
	}

	//Space is fully Occupied
	if(minOrder == MAX_HEAP_ORDER + 1)
	{
		return NULL;
	}

	//Slice our memory block
	while(minOrder >= order) 
	{
		//find upper location
		upperBound = (1 << minOrder);
		while(upperBound <= minOrderLocation)
		{
			upperBound = upperBound + (1 << minOrder);
		}
	
		if(minOrder != order)
		{
			printf("Created a left child node (order = %d, addr = %d, size = %d) of parent (order = %d, addr = %d, size = %d)\n"
			, minOrder - 1, minOrderLocation << HEAP_FIRST_BITNUM, (1 << (minOrder - 1)) << HEAP_FIRST_BITNUM
			, minOrder, minOrderLocation << HEAP_FIRST_BITNUM, (1 << minOrder) << HEAP_FIRST_BITNUM);		

			printf("Created a right child node (order = %d, addr = %d, size = %d) of parent (order = %d, addr = %d, size = %d)\n"
			, minOrder - 1, minOrderLocation << HEAP_FIRST_BITNUM, (1 << (minOrder)) << HEAP_FIRST_BITNUM
			, minOrder, minOrderLocation << HEAP_FIRST_BITNUM, (1 << minOrder) << HEAP_FIRST_BITNUM);
		}
		
		for(i = minOrderLocation; i < upperBound; ++i)
		{
			//Slice The Tree
			if(minOrder != order)
			{
				pcb->heap[i] = minOrder -1;
			}	
			else
			{
				pcb->heap[i] |= HEAP_BLOCK_USE_MASK;
			}
		}	
		minOrder--;
	}

	printf("Allocated the block: order = %d, addr = %d, requested mem size = %d, block size = %d\n", order, minOrderLocation << HEAP_FIRST_BITNUM, memsize, (1 << order) << HEAP_FIRST_BITNUM);
	return (void *) (pcb->heapBaseAddr + (minOrderLocation << HEAP_FIRST_BITNUM));
}

