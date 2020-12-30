/***************************************************************************

  z80bw.c

  Functions to emulate general aspects of the machine
  (RAM, ROM, interrupts, I/O ports)

***************************************************************************/

#include "driver.h"
#include "z80/z80.h"


int astinvad_interrupt(void)
{
	static int count;

	count++;

	if (count & 1) return 0x00cf;	/* RST 08h - 8080's IRQ */
	else
	{
		Z80_Regs R;

		Z80_GetRegs(&R);
	#ifndef USE_DRZ80 /* FRANXIS 01-09-2005 */
		R.IFF2 = 1;	/* enable interrupts */
	#else
		R.Z80IF = (R.Z80IF | 0x2); /* Set IFF2 */
	#endif
		Z80_SetRegs(&R);

		return 0x00d7;	/* RST 10h - 8080's NMI */
	}
}

