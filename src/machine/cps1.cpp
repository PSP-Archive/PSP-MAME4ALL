/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"

#include "drivers/cps1.h"

int cps1_ram_size;
unsigned char *cps1_ram;

int cps1_input_r(int offset)
{
       int control=readinputport (offset/2);
       return (control<<8) | control;
}

int cps1_player_input_r(int offset)
{
       return (readinputport (offset + 4 )+
		(readinputport (offset+1 + 4 )<<8));
}

int cps1_interrupt(void)
{
	return 6;
}

int cps1_interrupt2(void)
{
	if (cpu_getiloops() == 0)
	{
		return 2;
	}
	else return 4;
}


int cps1_interrupt3(void)
{
       return 2;
}
