#include "m68000.h"
#include "driver.h"
#include "cpuintrf.h"
#include "readcpu.h"
#include <stdio.h>
/* BFV 061298 - added include for instruction timing table */
#include "cycletbl.h"

/* LBO 090597 - added these lines and made them extern in cpudefs.h */
int movem_index1[256];
int movem_index2[256];
int movem_next[256];
UBYTE *actadr;

m68k_regstruct m68k_regs, lastint_m68k_regs;

union flagu intel_flag_lookup[256];
union flagu regflags;

extern int cpu_interrupt(void);
extern void BuildCPU(void);

#define MC68000_interrupt() (cpu_interrupt())

#define ReadMEM(A) (cpu_readmem24(A))
#define WriteMEM(A,V) (cpu_writemem24(A,V))

int MC68000_ICount;
int pending_interrupts;

static int InitStatus=0;

int opcode ;


void m68k_dumpstate(void)
{
}






static void initCPU(void)
{
    int i,j;

    for (i = 0 ; i < 256 ; i++) {
      for (j = 0 ; j < 8 ; j++) {
       if (i & (1 << j)) break;
      }
     movem_index1[i] = j;
     movem_index2[i] = 7-j;
     movem_next[i] = i & (~(1 << j));
    }
    for (i = 0; i < 256; i++) {
         intel_flag_lookup[i].flags.c = !!(i & 1);
         intel_flag_lookup[i].flags.z = !!(i & 64);
         intel_flag_lookup[i].flags.n = !!(i & 128);
         intel_flag_lookup[i].flags.v = 0;
    }

}

void Exception(int nr, CPTR oldpc)
{
//printf("Exception/IRQ(%i) -%i\n",nr-24,exception_cycles[nr]);
	MC68000_ICount -= exception_cycles[nr];

   MakeSR();

   if(!m68k_regs.s) {
   	  m68k_regs.usp=m68k_regs.a[7];
      m68k_regs.a[7]=m68k_regs.isp;
      m68k_regs.s=1;
   }

   m68k_regs.a[7] -= 4;
   put_long (m68k_regs.a[7], m68k_getpc ());
   m68k_regs.a[7] -= 2;
   put_word (m68k_regs.a[7], m68k_regs.sr);
   m68k_setpc(get_long(m68k_regs.vbr + 4*nr));

   m68k_regs.t1 = m68k_regs.t0 = m68k_regs.m = 0;
}

INLINE void Interrupt68k(int level)
{
   int ipl=m68k_regs.intmask;
   if(level>ipl)
   {
   	Exception(24+level,0);
   	pending_interrupts &= ~(1 << (level + 24));
   	m68k_regs.intmask = level;
   	MakeSR();
   }
}

void Initialisation(void) {
   /* Init 68000 emulator */
   BuildCPU();
   initCPU();
}

void MC68000_Reset(void)
{
if (!InitStatus)
{
	Initialisation();
	InitStatus=1;
}

   m68k_regs.a[7]=get_long(0);
   m68k_setpc(get_long(4));

   m68k_regs.s = 1;
   m68k_regs.m = 0;
   m68k_regs.stopped = 0;
   m68k_regs.t1 = 0;
   m68k_regs.t0 = 0;
   ZFLG = CFLG = NFLG = VFLG = 0;
   m68k_regs.intmask = 7;
   m68k_regs.vbr = m68k_regs.sfc = m68k_regs.dfc = 0;
   m68k_regs.fpcr = m68k_regs.fpsr = m68k_regs.fpiar = 0;

   pending_interrupts = 0;
}


void MC68000_SetRegs(MC68000_Regs *src)
{

	m68k_regs = src->m68k_regs ;

{	int sr = m68k_regs.sr ;

	NFLG = (sr >> 3) & 1;
	ZFLG = (sr >> 2) & 1;
	VFLG = (sr >> 1) & 1;
	CFLG = sr & 1;
	pending_interrupts = src->pending_interrupts;
}}

void MC68000_GetRegs(MC68000_Regs *dst)
{
   MakeSR();
	m68k_regs.sr = (m68k_regs.sr & 0xfff0) | (NFLG << 3) | (ZFLG << 2) | (VFLG << 1) | CFLG;
	dst->m68k_regs = m68k_regs;
	dst->pending_interrupts = pending_interrupts;
}


void MC68000_Cause_Interrupt(int level)
{
//printf("INT %i\n",level);
	if (level >= 1 && level <= 7)
		pending_interrupts |= 1 << (24 + level);
}


void MC68000_Clear_Pending_Interrupts(void)
{
	pending_interrupts &= ~0xff000000;
}


int  MC68000_GetPC(void)
{
	return m68k_regs.pc;
}



/* Execute one 68000 instruction */

int MC68000_Execute(int cycles)
{
	if ((pending_interrupts & MC68000_STOP) && !(pending_interrupts & 0xff000000))
		return cycles;

	MC68000_ICount = cycles;
	do
	{
		if (pending_interrupts & 0xff000000)
		{
			int level, mask = 0x80000000;
			for (level = 7; level; level--, mask >>= 1)
				if (pending_interrupts & mask)
					break;
			Interrupt68k (level);
		}

		{	/* ASG 980413 */
			extern int previouspc;
			previouspc = m68k_regs.pc;
		}

		#ifdef ASM_MEMORY
			opcode=nextiword_opcode();
		#else
			opcode=nextiword();
		#endif

		/* BFV 061298 - removed "MC68000_ICount -= 12;" added index lookup for timing table */
		MC68000_ICount -= cycletbl[opcode];
		cpufunctbl[opcode]();

	}
	while (MC68000_ICount > 0);

        return (cycles - MC68000_ICount);
}








UBYTE *allocmem(long size, UBYTE init_value) {
   UBYTE *p;
   p=(UBYTE *)gp2x_malloc(size);
   return(p);
}


