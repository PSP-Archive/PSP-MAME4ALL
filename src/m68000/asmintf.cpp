/*
	Interface routine for 68kem <-> Mame

*/

#include "driver.h"
#include "M68000.H"

#ifdef WIN32
#define CONVENTION __cdecl
#else
#define CONVENTION
#endif

extern void CONVENTION M68KRUN(void);
extern void CONVENTION M68KRESET(void);


/********************************************/
/* Interface routines to link Mame -> 68KEM */
/********************************************/

void MC68000_Reset(void)
{
	fast_memset(&m68k_regs,0,sizeof(m68k_regs));

    m68k_regs.a[7] = m68k_regs.isp = get_long(0);
    m68k_regs.pc   = get_long(4) & 0xffffff;
    m68k_regs.sr_high = 0x27;
   	m68k_regs.sr = 0x2700;

    M68KRESET();
}


int  MC68000_Execute(int cycles)
{
	if (m68k_regs.IRQ_level == 0x80) return cycles;		/* STOP with no IRQs */

  	MC68000_ICount = cycles;

	M68KRUN();

    return (cycles - MC68000_ICount);
}


static m68k_regstruct Mym68k_regs;

void MC68000_SetRegs(MC68000_Regs *src)
{
 	m68k_regs = src->m68k_regs;
}

void MC68000_GetRegs(MC68000_Regs *dst)
{
  	dst->m68k_regs = m68k_regs;
}

void MC68000_Cause_Interrupt(int level)
{
	if (level >= 1 && level <= 7)
        m68k_regs.IRQ_level |= 1 << (level-1);
}

void MC68000_Clear_Pending_Interrupts(void)
{
	m68k_regs.IRQ_level = 0;
}

int  MC68000_GetPC(void)
{
    return m68k_regs.pc;
}

