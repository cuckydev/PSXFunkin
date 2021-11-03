/*
	"mem" by Regan "CuckyDev" Green:
	A tiny portable C89 memory allocator.
	
	This is a single-header library. You must include this file alongside `#define MEM_IMPLEMENTATION` in one file in order to use.
	You can then include `mem.h` in other files for the function declarations.
	
	Additional control defines:
	MEM_STAT - This will enable the Mem_GetStat function which returns information about available memory in the heap.
*/

#ifndef MEM_GUARD_MEM_H
#define MEM_GUARD_MEM_H

#include <stddef.h>

/* Constants and macros */
#define MEM_ALIGNSIZE 0x10
#define MEM_ALIGN(x) (((size_t)(x) + 0xF) & ~0xF)

#ifdef PSXF_STDMEM

#include <stdlib.h>

#undef MEM_STAT /* Control unsupported */

#define Mem_Init(x,y)
#define Mem_Alloc malloc
#define Mem_Free free

#else

/* Function declarations */
int Mem_Init(void *ptr, size_t size);
void *Mem_Alloc(size_t size);
void Mem_Free(void *ptr);
#ifdef MEM_STAT
	void Mem_GetStat(size_t *used, size_t *size, size_t *max);
#endif

/* Implementation */
#ifdef MEM_IMPLEMENTATION

typedef struct Mem_Header
{
	struct Mem_Header *prev, *next;
	size_t size;
} Mem_Header;
#define MEM_HEDSIZE (MEM_ALIGN(sizeof(Mem_Header)))

static Mem_Header *mem = NULL;
#ifdef MEM_STAT
	static size_t mem_used, mem_max;
#endif

int Mem_Init(void *ptr, size_t size)
{
	/* Make sure there's enough space for mem header */
	if (ptr == NULL || size < MEM_HEDSIZE)
		return 1;
	
	/* Get mem pointer and available range (after 16 byte alignment) */
	mem = (Mem_Header*)MEM_ALIGN(ptr);
	
	/* Initial mem header */
	mem->prev = NULL;
	mem->next = NULL;
	mem->size = ((char*)ptr + size) - (char*)mem;
	
	/* Initial mem state */
	#ifdef MEM_STAT
		mem_max = mem_used = MEM_HEDSIZE;
	#endif
	
	return 0;
}

static Mem_Header *Mem_GetHeader(void *ptr)
{
	if (ptr == NULL)
		return NULL;
	return (Mem_Header*)((char*)ptr - MEM_HEDSIZE);
}

void *Mem_Alloc(size_t size)
{
	/* Ensure we have a heap */
	if (mem == NULL)
		return NULL;
	
	/* Get true size we have to fit */
	size = MEM_ALIGN(size + MEM_HEDSIZE);
	
	/* Get header pointer */
	Mem_Header *head, *prev, *next;
	char *hpos = (char*)mem + MEM_HEDSIZE;
	
	prev = mem;
	next = prev->next;
	
	while (1)
	{
		if (next != NULL)
		{
			/* Check against the next block */
			size_t cleft = (char*)next - hpos;
			if (cleft >= size)
			{
				/* Set pointer */
				head = (Mem_Header*)hpos;
				break;
			}
			
			/* Check next header */
			hpos = (char*)next + next->size;
			prev = next;
			next = prev->next;
		}
		else
		{
			/* Check against end of heap */
			size_t cleft = ((char*)mem + mem->size) - hpos;
			if (cleft < size)
				return NULL;
			
			/* Set pointer */
			head = (Mem_Header*)hpos;
			break;
		}
	}
	
	/* Link header */
	head->size = size;
	head->prev = prev;
	if ((head->next = prev->next) != NULL)
		head->next->prev = head;
	prev->next = head;
	
	#ifdef MEM_STAT
		/* Update stats */
		if ((mem_used += size) >= mem_max)
			mem_max = mem_used;
	#endif
	
	return (void*)(hpos + MEM_HEDSIZE);
}

void Mem_Free(void *ptr)
{
	/* Get header of pointer */
	if (ptr == NULL)
		return;
	Mem_Header *head = Mem_GetHeader(ptr);
	
	/* Unlink header */
	if ((head->prev->next = head->next) != NULL)
		head->next->prev = head->prev;
	
	#ifdef MEM_STAT
		/* Update stats */
		mem_used -= head->size;
	#endif
}

#ifdef MEM_STAT
	void Mem_GetStat(size_t *used, size_t *size, size_t *max)
	{
		if (used != NULL)
			*used = mem_used;
		if (size != NULL)
			*size = mem->size;
		if (max != NULL)
			*max = mem_max;
	}
#endif

#endif /* MEM_IMPLEMENTATION */

#endif /* PSXF_STDMEM */

#endif /* MEM_GUARD_MEM_H */
