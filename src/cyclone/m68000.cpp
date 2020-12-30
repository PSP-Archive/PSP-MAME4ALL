#include "m68000.h"
#include "driver.h"
#include "cpuintrf.h"

static int pending_interrupts;

struct Cyclone cyclone;

static unsigned int MyCheckPc(unsigned int pc)
{
  	pc = (pc-cyclone.membase) & 0xffffff; /* Get the real program counter */
	change_pc24(pc);
	cyclone.membase=(unsigned int)OP_ROM;
  	return cyclone.membase+pc; /* New program counter */
}

static void update_irq_line(void)
{
	if (pending_interrupts & 0xff000000)
	{
		int level, mask = 0x80000000;
		for (level = 7; level; level--, mask >>= 1)
			if (pending_interrupts & mask)
				break;
		cyclone.irq = level;
	}
	else
	{
		cyclone.irq = 0;
	}
}

static int MyIrqCallback(int irq)
{
	pending_interrupts &= ~(1 << (24 + irq));
	update_irq_line();
	return CYCLONE_INT_ACK_AUTOVECTOR;
}

void MC68000_Reset(void)
{
   	CycloneInit();
	fast_memset(&cyclone, 0, sizeof(cyclone));
	cyclone.checkpc=MyCheckPc;
	/* some params / return values have different size,
	 * but they are compatible in ARM arch. */
	cyclone.read8   = (unsigned int (*)(unsigned int)) cpu_readmem24;
	cyclone.read16  = (unsigned int (*)(unsigned int)) cpu_readmem24_word;
	cyclone.read32  = (unsigned int (*)(unsigned int)) cpu_readmem24_dword;
	cyclone.write8  = (void (*)(unsigned int, unsigned char))  cpu_writemem24;
	cyclone.write16 = (void (*)(unsigned int, unsigned short)) cpu_writemem24_word;
	cyclone.write32 = (void (*)(unsigned int, unsigned int))   cpu_writemem24_dword;
	cyclone.fetch8  = (unsigned int (*)(unsigned int)) cpu_readmem24;
	cyclone.fetch16 = (unsigned int (*)(unsigned int)) cpu_readmem24_word;
	cyclone.fetch32 = (unsigned int (*)(unsigned int)) cpu_readmem24_dword;
	cyclone.IrqCallback=MyIrqCallback;
  	cyclone.srh=0x27; /* Set supervisor mode */
  	cyclone.a[7]=cyclone.read32(0); /* Get Stack Pointer */
  	cyclone.membase=0;
  	cyclone.pc=cyclone.checkpc(cyclone.read32(4)); /* Get Program Counter */
	cyclone.state_flags = 0; /* not stopped or tracing */
   	pending_interrupts = 0;
}

int  MC68000_GetPC(void)
{
	return (cyclone.pc - cyclone.membase) & 0xffffff;
}

void MC68000_SetRegs(MC68000_Regs *src)
{
	fast_memcpy(&cyclone,&(src->regs),sizeof(cyclone));
	pending_interrupts = src->pending_interrupts;
	update_irq_line();
}

void MC68000_GetRegs(MC68000_Regs *dst)
{
	fast_memcpy(&(dst->regs),&cyclone,sizeof(cyclone));
	dst->pending_interrupts = pending_interrupts;
}

void MC68000_Cause_Interrupt(int level)
{
	if (level >= 1 && level <= 7)
		pending_interrupts |= 1 << (24 + level);
	update_irq_line();
}


void MC68000_Clear_Pending_Interrupts(void)
{
	pending_interrupts &= ~0xff000000;
	cyclone.irq=0;
}


int MC68000_Execute(int cycles)
{
	cyclone.cycles = cycles;
	CycloneRun(&cyclone);
   	return (cycles - cyclone.cycles);
}
