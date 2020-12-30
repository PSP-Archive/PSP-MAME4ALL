#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#include "driver.h"
#include "cpuintrf.h"
#include "memory.h"
#include "z80/z80.h"
#include "cz80.h"

// #include "hack.h"

cz80_struc *mame4all_cz80_struc;

#include "memory.h"

void Z80_Reset(Z80_DaisyChain *daisy_chain)
{
	unsigned *ctx=(unsigned *)(((unsigned)cpu_getcontext(cpu_getactivecpu())));
printf("Z80_Reset %i\n",cpu_getactivecpu());
if (daisy_chain)
puts(" Caution !!! Z80_DaisyChain not implementated yet !!!");
	mame4all_cz80_struc=(cz80_struc *)&ctx[4];

	Cz80_Init(mame4all_cz80_struc);
	Cz80_Reset(mame4all_cz80_struc);

	ctx[0]=(unsigned)Cz80_Get_PC(mame4all_cz80_struc);
}

void Z80_GetRegs(Z80_Regs *Regs)
{
	unsigned *ctx=(unsigned *)Regs;
	ctx[0] = (unsigned)Cz80_Get_PC(mame4all_cz80_struc);
}

void Z80_SetRegs (Z80_Regs *Regs)
{
	unsigned *ctx=(unsigned *)Regs;
	mame4all_cz80_struc=(cz80_struc *)&ctx[4];
	Cz80_Set_PC(mame4all_cz80_struc,ctx[0]);
}

unsigned Z80_GetPC (void)
{
	return Cz80_Get_PC(mame4all_cz80_struc);
}

void Z80_Cause_Interrupt(int type)
{
	if (type == Z80_NMI_INT)
		Cz80_Set_NMI(mame4all_cz80_struc);
	else if (type >= 0)
		Cz80_Set_IRQ(mame4all_cz80_struc, type & 0xff);
}

void Z80_Clear_Pending_Interrupts(void)
{
	Cz80_Clear_IRQ(mame4all_cz80_struc);
	Cz80_Clear_NMI(mame4all_cz80_struc);
}


int Z80_Execute(int cycles)
{
/*
static int cuantos=0;
printf("%i Z80_Execute(%i)\n",cuantos++,cycles);
	int falta=cycles;
	do{
		unsigned short nowpc=Cz80_Get_PC(mame4all_cz80_struc);
printf("PC=%.4X (%.2X%.2X%.2X%.2X)/(%.2X%.2X%.2X%.2X)\n",nowpc,OP_ROM[nowpc+0],OP_ROM[nowpc+1],OP_ROM[nowpc+2],OP_ROM[nowpc+3],OP_RAM[nowpc+0],OP_RAM[nowpc+1],OP_RAM[nowpc+2],OP_RAM[nowpc+3]);
//		___z80_hack(nowpc);
//		falta-=Cz80_Exec(mame4all_cz80_struc,1);
		Cz80_Exec(mame4all_cz80_struc,1);
		falta-=6;
	}while(falta>0);
puts("----");
if (cuantos>100) exit(0);
return cycles;
//	return cycles-falta;
*/
	return Cz80_Exec(mame4all_cz80_struc,cycles);
}

