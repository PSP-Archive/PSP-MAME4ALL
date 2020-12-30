
#include "minimal.h"

#define UPPERMEM_BLOCKSIZE 1024
#define UPPERMEM_START 0x2000000
#define UPPERMEM_SIZE 0x2000000
struct memmalloc
{
	unsigned int	mem;
	int	size;
};

static memmalloc mmX[4096];

static void *uppermem;
static int uppermap[UPPERMEM_SIZE/UPPERMEM_BLOCKSIZE];
static int memused = 0;

void malloc_debug_init(void)
{
	fast_memset(mmX, 0, sizeof(mmX));
}

void malloc_debug_status(int flag)
{
	int total = 0;
	int items = 0;
	int i;
	for (i=0; i<4096; i++)
	{
		if (mmX[i].mem)
		{
			total += mmX[i].size;
			items++;
			if (flag)
				printf("memory: %d size: %d\n", mmX[i].mem, mmX[i].size);
		}
	}
	printf ("malloc() total %d in %d items\n", total, items);
}


static void malloc_mmX(unsigned int addr, int size)
{
	int i;
	for (i=0; i<4096; i++)
	{
		if (mmX[i].mem) continue;
		mmX[i].mem = addr;
		mmX[i].size = size;
		break;
	}
}
static int free_mmX(unsigned int addr)
{
	int i;
	int size;
	for (i=0; i<4096; i++)
	{
		if (mmX[i].mem != addr) continue;
		size = mmX[i].size;
		mmX[i].mem = 0;
		mmX[i].size = 0;
		return 1;
	}
	printf("ERROR: Could not find address: %d\n", addr);
	return 1;
}

static void * upper_malloc(size_t size)
{
  	int i=0,j=0;
 	void *mem = NULL;
 	int BSize = ((size-1)/UPPERMEM_BLOCKSIZE) + 1;

  	while (i<UPPERMEM_SIZE/UPPERMEM_BLOCKSIZE)
  	{
		for(j=0;j<BSize;j++)
  		{
    			if (uppermap[i+j])
    			{
      				i+=j;
      				i+=uppermap[i];
      				break;
    			}
  		}
		if (j==BSize) {
			uppermap[i] = BSize;
  			mem = ((char*)uppermem) + (i*UPPERMEM_BLOCKSIZE);
  			//fast_memset(mem, 0, size);
  			return mem;
  		}
	}
	return NULL;
}

void gp2x_malloc_init(void *pointer)
{
	uppermem=pointer;
	//malloc_debug_init();
	//fast_memset(uppermap,0,sizeof(uppermap));
}

void * upper_take(int Start, size_t Size)
{
	uppermap[(Start - UPPERMEM_START) / UPPERMEM_BLOCKSIZE] = ((Size-1)/UPPERMEM_BLOCKSIZE) + 1;
  	void* mem = ((char*)uppermem) + (Start-UPPERMEM_START);
	return(mem);
}


void * gp2x_mallocX(size_t size, const char *func)
{
	void *ptr=NULL;
	if (size>=(8*1024*1024)) {
		ptr=upper_malloc(size);
		if (ptr)
		{
//printf ("%s upper malloc %d at %d\n", func, size, ptr);
			return (ptr);
		}
	}
	ptr=malloc(size);
	if (ptr)
	{
		memused += size;
		malloc_mmX((int)ptr, size);
//printf ("%s malloc %d at %d\n", func, size, ptr);
		return (ptr);
	}
	ptr=upper_malloc(size);
	if (ptr)
	{
		memused += size;
		malloc_mmX((int)ptr, size);
//printf ("upper malloc %d at %d\n", size, ptr);
		return (ptr);
	}
	printf("Could not malloc %d\n", size);
	return (NULL);
}

void gp2x_freeX(void *ptr, const char *func)
{
//printf ("%s free at %d\n", func, ptr);

	free_mmX((int)ptr);
  	int i = (((int)ptr) - ((int)uppermem));
  	if (i < 0 || i >= UPPERMEM_SIZE) {
  		if (ptr) free(ptr);
  	} else {
		uppermap[i/UPPERMEM_BLOCKSIZE] = 0;
	}
	ptr=NULL;
}
