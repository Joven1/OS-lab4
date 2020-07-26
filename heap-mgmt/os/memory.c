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

int get_Buddy_Index(int index, int order)
{
	int index_test = index; //Copy of the index
	int buddy_index; //Buddy Node Index
	int i; //Loop Control Variable
	//We can see which index is the buddy index by using the fact that the even numbered indexes are paired with the right, and odd
	//numbered indexes are paired with the left
	for(i = 0; i < order; i++)
	{
		index_test = index_test / 2;
	}	
	if(index_test % 2 == 0) //Even
	{
		buddy_index = index + (1 << order);
	}
	else //Odd
	{
		buddy_index = index - (1 << order);
	}
	return buddy_index;	
}

void print_Coalesced_Nodes(Heap_Node * current, Heap_Node * buddy)
{
	uint32 free_address = (current->address < buddy->address) ? current->address : buddy->address;
	
	printf("Coalesced buddy nodes ");
	
	if(current->address < buddy->address)
	{
		printf("(order = %d, addr = %d, size = %d) & (order = %d, addr = %d, size = %d)\n"
			, current->order, current->address, current->size, buddy->order, buddy->address, buddy->size);
	}
	else
	{
		printf("(order = %d, addr = %d, size = %d) & (order = %d, addr = %d, size = %d)\n"
			, buddy->order, buddy->address, buddy->size, current->order, current->address, current->size);
	}
	
	printf("into the parent node (order = %d, addr = %d, size = %d)\n", buddy->order + 1, free_address, current->size + buddy->size);

}


void Merge_Nodes(Heap_Node * heap, uint32 current_index)
{	
	uint32 buddy_index = get_Buddy_Index(current_index, heap[current_index].order);
	uint32 index = current_index < buddy_index ? current_index: buddy_index; //Loop Control Variable
	uint32 upper_Bound = heap[current_index].size + heap[buddy_index].size; //Upper Bound we have to set 
	uint32 i;
	
	//Merged Node 
	Heap_Node merged_Node;
	merged_Node.size = heap[index].size * 2;
	merged_Node.order = heap[index].order + 1;
	merged_Node.address = heap[index].address;
	merged_Node.occupied = false;

	//Conditions where we should stop merging
	if(heap[index].order == 7) //If our heap is at the max order
	{
		return;
	}
	else if(heap[buddy_index].occupied == true) //If the other heap has a used block
	{
		return;
	}
	else if(heap[buddy_index].size != heap[current_index].size) //If the sizes of the heaps are not equal
	{
		return;
	}
	
	print_Coalesced_Nodes(&heap[current_index], &heap[buddy_index]);
	for(i = index; i < (index + upper_Bound/32); i++)
	{
		heap[i] = merged_Node;
	}
	//Recursively Merge the Nodes again
	Merge_Nodes(heap, index);
}


int mfree(PCB* pcb, void* ptr) 
{
	int index = ((int) ptr - pcb->heapBaseAddr) / 32; //Obtain the heap location
	int current_index = index; //Index of memory to free in the heap
	int buddy_index; //Index of the buddy to the memory occupied
	Heap_Node current_Node; //Node to Free
	Heap_Node buddy_Node; //Buddy to Node to Free
	int i; //Loop control Variable
	uint32 Node_Order; //Order of the Node to free
	uint32 bytes_freed;
	Heap_Node * heap = pcb->Heap;

	if( !heap[current_index].occupied ) //If the chunk of memory was already free, return -1
	{
		return -1;
	}
	
	if(index < 0) //Invalid Pointer
	{
		return -1;
	}

	if(index > (MAX_HEAP_BLOCKS - 1)) //Invalid Pointer
	{
		return -1;
	}


	//Free the Current Node
	current_Node = heap[current_index];
	Node_Order = heap[current_index].order;
	bytes_freed = current_Node.size;
	for(i = current_index; i < (current_index + (1 << Node_Order)); i++)
	{
		heap[i].occupied = false;
	}
	printf("Freed the block: order = %d, addr = %d, size = %d\n", current_Node.order, current_Node.address, current_Node.size);
	Merge_Nodes(heap, current_index);
	return bytes_freed; //Return the number of bytes freed

}


void HeapInit(PCB * pcb)
{
	int i;
	Heap_Node * heap = pcb->Heap;
	for(i = 0; i < MAX_HEAP_BLOCKS; i++)
	{
		pcb->Heap[i].order = MAX_HEAP_ORDER; //Max order is 7 -> 4096
		pcb->Heap[i].address = 0x0; //The Heap's address always starts out at zero
		pcb->Heap[i].size = 32 * (1 << MAX_HEAP_ORDER); //Size starts out as 4096
		pcb->Heap[i].occupied = false; //Heap Node is open at first
	}

}

void printHeap(Heap_Node * heap)
{
	int i = 0;
	while(i < MAX_HEAP_BLOCKS)
	{
		printf("|Index: %d Size: %d Status: %d  |", i, heap[i].size, heap[i].occupied);
		i = i + (1 << heap[i].order);
	}
	printf("\n");
}

void printBinary(uint32 n)
{
	if(n)
	{
		printBinary(n >> 1);
		printf("%d", n & 1);
	}
}

void SplitNode(PCB * pcb, uint32 index, uint32 order, uint32 memsize)
{
	Heap_Node * heap = pcb->Heap;
	
	//Determine how many times to split 
	uint32 times_split = heap[index].order - order;
 	uint32 i; //Loop control variable
	uint32 first_half; //Index of the left child
	uint32 second_half; //Index of the right child
	
	uint32 first_half_address; //Left child's address
	uint32 second_half_address; //Right child's address

	uint32 half_size; //Child's size
	uint32 half_order; //Child's order
	

	uint32 parent_order; //Order before splitting a node
	uint32 parent_size; //Size before splitting a node
	uint32 parent_address; //Address before splitting a node

	
	//Split the memory n number of times specified
	for(i = 0; i < times_split; i++)
	{
		//Extract the parent Node information from the heap
		parent_order = heap[index].order;
		parent_size =  heap[index].size;
		parent_address = heap[index].address;

		//Extract the left child's information
		first_half = index; //Our first half of the index starts at the beginning
		first_half_address = heap[index].address;

		//Obtain the sizes and orders of the children
		half_size = heap[index].size / 2; //Size is cut in half every order iteration		
		half_order = heap[index].order - 1; //Every order iteration, order is shrinked
		heap[index].order--;	


		second_half = index + (1 << half_order); //Second half of the block is at the beginning + order size
		second_half_address = heap[index].address + 32 * (1 << half_order); //Address is the base index address + size

		//Set the left child
		for(; first_half <  (index + (1 << half_order)); first_half++) //Loop through the first half of the block
		{
			//Set the new parameters for the child
			heap[first_half].size = half_size;
			heap[first_half].order = half_order;
			heap[first_half].Buddy = &heap[second_half];	
			heap[first_half].address = first_half_address;
		}
	
		//Set the right child	
		for(; second_half < (index + 2 * (1 << half_order)); second_half++) //Loop through the second half of the block
		{
			//Set the new parameters for the child
			heap[second_half].size = half_size;
			heap[second_half].order = half_order;
			heap[second_half].Buddy = &heap[index];	
			heap[second_half].address = second_half_address;
		}

		//Reset both pointers to the children for printing
		second_half = index + (1 << half_order);
		first_half = index;
		
		printf("Created a right child node (order = %d, addr = %d, size = %d ) of parent (order = %d, addr = %d, size = %d)\n",
		heap[second_half].order, heap[second_half].address, heap[second_half].size, parent_order, parent_address, parent_size); 
		
		
		printf("Created a left child node (order = %d, addr = %d, size = %d ) of parent (order = %d, addr = %d, size = %d)\n",
		heap[first_half].order, heap[first_half].address, heap[first_half].size, parent_order, parent_address, parent_size); 

	
	}
}

void printAllocation(uint32 order, uint32 address, uint32 memory_size, uint32 block_size)
{
	printf("Allocated the block: order = %d, addr = %d, requested mem size = %d, block size = %d\n", order, address, memory_size, block_size);	
}

void printHeapResult(uint32 size, uint32 virtual_address, uint32 physical_address)
{
		printf("Created a heap block of size %d bytes: virtual address %d, physical address %d\n", size, virtual_address, physical_address);		
}
void * malloc(PCB * pcb, int memsize)
{
	uint32 i; //Loop Control Variable
	uint32 order = 0; //Order the memsize falls under
	uint32 mem_block = 32; //Our block of memory first starts off as order 0
	uint32 smallest_Order = MAX_HEAP_ORDER + 1; 
	uint32 smallest_Order_index = 0; //Variable to keep track of smallest block to hold memory
	Heap_Node * heap = pcb->Heap; //Pointer to the PCB's heap	
	uint32 virtual_address; //Virtual Address of the Heap
	uint32 physical_address; //Address in Memory

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

	//In our first iteration, try finding the exact fit for our memory
	for(i = 0; i < MAX_HEAP_BLOCKS; i += (1 << heap[i].order)) //Traverse the Heap, We can skip over indexes based on how large the order is
	{
		
		if( !(heap[i].occupied) ) //If we find a slot that is both equal to the same order and it is not occupied occupy it
		{
			if(heap[i].order == order) //If the order is equal allocate that block
			{
				heap[i].occupied = true;
				printAllocation(heap[i].order, heap[i].address, memsize, heap[i].size);			
				
				virtual_address = pcb->heapBaseAddr + heap[i].address;
				physical_address = MemoryTranslateUserToSystem(pcb, virtual_address);

				printHeapResult(heap[i].size, virtual_address, physical_address);			
							
				return (void *) virtual_address;
			}
			else if(heap[i].order > order) //If the order can at least hold the block
			{
				if(heap[i].order < smallest_Order) //If the order is smaller than the minimum block
				{
					smallest_Order = heap[i].order;
					smallest_Order_index = i;
				}	
			}
		}
	}
	
	if(smallest_Order == (MAX_HEAP_ORDER + 1) ) //No Blocks are available
	{
		return NULL;
	}
	
	//Split the node 
	//For some reason, if you print before the declaration of split Node a segmentation fault will occur	
	SplitNode(pcb, smallest_Order_index, order, memsize);


	//To combat this, I just recursively call malloc again so it hits the upper case. It has a max stack of 2, since we split the nodes early
	return malloc(pcb, memsize);

}
