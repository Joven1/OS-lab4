#ifndef	_memory_h_
#define	_memory_h_

// Put all your #define's in memory_constants.h
#include "memory_constants.h"

extern int lastosaddress; // Defined in an assembly file

//--------------------------------------------------------
// Existing function prototypes:
//--------------------------------------------------------

int MemoryGetSize();
void MemoryModuleInit();
uint32 MemoryTranslateUserToSystem (PCB *pcb, uint32 addr);
int MemoryMoveBetweenSpaces (PCB *pcb, unsigned char *system, unsigned char *user, int n, int dir);
int MemoryCopySystemToUser (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryCopyUserToSystem (PCB *pcb, unsigned char *from, unsigned char *to, int n);
int MemoryPageFaultHandler(PCB *pcb);

//---------------------------------------------------------
// Put your function prototypes here
//---------------------------------------------------------
// All function prototypes including the malloc and mfree uu
int MemoryAllocPage(void); 
uint32 MemorySetupPte (uint32 page);

//Forking Funcitons
void MemorySharePte(uint32 pte);
uint32 Set_ReadOnly_Bit(uint32 page);
uint32 Unset_ReadOnly_Bit(uint32 page);
void Memory_ROP_ACCESS_handler(PCB * currentPCB);

void MemoryFreePage(uint32 page);
void PrintPageTable(PCB * pcb);

int mfree(PCB* pcb, void* ptr);
void * malloc(PCB * pcb, int memsize);

#endif	// _memory_h_
