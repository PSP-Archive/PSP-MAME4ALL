#include "driver.h"
#include "cpuintrf.h"
#include "z80.h"

#define INT_IRQ 0x01
#define NMI_IRQ 0x02

Z80_Regs Z80;
extern int previouspc;

static unsigned int z80_rebasePC(unsigned short address)
{
	change_pc(address);
	Z80.Z80PC_BASE = (unsigned int) OP_ROM;
	Z80.Z80PC = Z80.Z80PC_BASE + address;
	return (Z80.Z80PC);
}

static unsigned int z80_rebaseSP(unsigned short address)
{
	change_pc(address);	
	Z80.Z80SP_BASE=(unsigned int) OP_ROM;
	Z80.Z80SP=Z80.Z80SP_BASE + address;
	return (Z80.Z80SP);
}

static unsigned short z80_read16(unsigned short Addr)
{
	return (((cpu_readmem16(Addr))&0xff)|((cpu_readmem16(Addr+1)&0xff)<<8));
}

static void z80_write8(unsigned char Value,unsigned short Addr)
{
	cpu_writemem16(Addr,Value);
}

static void z80_write16(unsigned short Value,unsigned short Addr)
{
	cpu_writemem16(Addr,(unsigned char)Value&0xff);
	cpu_writemem16(Addr+1,((unsigned char)(Value>>8))&0xff);
}

static void z80_irq_callback(void)
{
	Z80.Z80_IRQ = 0x00;
	previouspc=0;
}

/****************************************************************************
 * Reset registers to their initial values
 ****************************************************************************/
void Z80_Reset(Z80_DaisyChain *daisy_chain)
{
	fast_memset (&Z80, 0, sizeof(Z80_Regs));
  	Z80.z80_rebasePC=z80_rebasePC;
  	Z80.z80_rebaseSP=z80_rebaseSP;
  	Z80.z80_read8   =(unsigned char (*)(unsigned short))cpu_readmem16;
  	Z80.z80_read16  =z80_read16;
  	Z80.z80_write8  =z80_write8;
  	Z80.z80_write16 =z80_write16;
  	Z80.z80_in      =(unsigned char (*)(unsigned short))cpu_readport;
  	Z80.z80_out     =(void (*)(unsigned short, unsigned char))cpu_writeport;
  	Z80.z80_irq_callback=z80_irq_callback;
  	Z80.Z80IX = 0xffff<<16;
  	Z80.Z80IY = 0xffff<<16;
  	Z80.Z80F = (1<<2); /* set ZFlag */
	previouspc=0;
	Z80_Clear_Pending_Interrupts();
  	Z80.Z80SP=Z80.z80_rebaseSP(0xf000);
  	Z80.Z80PC=Z80.z80_rebasePC(0);
}

void Z80_Cause_Interrupt(int type)
{
    	if (type == Z80_NMI_INT) {
	       	Z80.pending_irq |= NMI_IRQ;
	} else if (type != Z80_IGNORE_INT) {
		Z80.z80irqvector = type & 0xff;
		Z80.pending_irq |= INT_IRQ;
    	}
}

void Z80_Clear_Pending_Interrupts(void)
{
    	Z80.pending_irq = 0;
}

/* Set all registers to given values */
void Z80_SetRegs (Z80_Regs *Regs)
{
	fast_memcpy(&Z80,Regs,sizeof(Z80));
	change_pc(Z80.Z80PC - Z80.Z80PC_BASE);
}

/* Get all registers in given buffer */
void Z80_GetRegs (Z80_Regs *Regs)
{
	fast_memcpy(Regs,&Z80,sizeof(Z80));
}

/* Return program counter */
unsigned Z80_GetPC (void)
{
	return (Z80.Z80PC - Z80.Z80PC_BASE);
}

#ifdef __cplusplus
extern "C" {
#endif
void Interrupt(void)
{
	if (Z80.pending_irq & NMI_IRQ) { /* NMI IRQ */
		previouspc = -1;
		Z80.Z80_IRQ = NMI_IRQ;
		Z80.pending_irq &= ~NMI_IRQ;
	} else if (Z80.Z80IF & 1) { /* INT IRQ and Interrupts enabled */
		previouspc = -1;
		Z80.Z80_IRQ = INT_IRQ;
		Z80.pending_irq &= ~INT_IRQ;
	}
}
#ifdef __cplusplus
} /* End of extern "C" */
#endif

/****************************************************************************
 * Execute IPeriod T-states. Return number of T-states really executed
 ****************************************************************************/
int Z80_Execute(int cycles)
{
	Z80.cycles = cycles;
	DrZ80Run(&Z80, cycles);
	change_pc(Z80.Z80PC - Z80.Z80PC_BASE);
	return (cycles-Z80.cycles);
}

void fake_z80_freemem(void)
{
}
