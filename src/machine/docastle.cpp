/***************************************************************************

  machine.c

  Functions to emulate general aspects of the machine (RAM, ROM, interrupts,
  I/O ports)

***************************************************************************/

#include "driver.h"
#include "timer.h"
#include "z80/z80.h"



static unsigned char buffer0[9],buffer1[9];



int docastle_shared0_r(int offset)
{
	/* this shouldn't be done, however it's the only way I've found */
	/* to make dip switches work in Do Run Run. */
	if (offset == 8)
	{
		cpu_cause_interrupt(1,Z80_NMI_INT);
		cpu_spinuntil_trigger(500);
	}

	return buffer0[offset];
}


int docastle_shared1_r(int offset)
{
	return buffer1[offset];
}


void docastle_shared0_w(int offset,int data)
{

	buffer0[offset] = data;

	if (offset == 8)
		/* awake the master CPU */
		cpu_trigger(500);
}


void docastle_shared1_w(int offset,int data)
{
	buffer1[offset] = data;

	if (offset == 8)
	{
		/* freeze execution of the master CPU until the slave has used the shared memory */
		cpu_spinuntil_trigger(500);
	}
}



void docastle_nmitrigger(int offset,int data)
{
	cpu_cause_interrupt(1,Z80_NMI_INT);
}
