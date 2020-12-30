/*****************************************************************************

	h6280.cpp - Portable Hu6280 emulator

	Copyright (c) 1999 Bryan McPhail, mish@tendril.force9.net

	This source code is based (with permission!) on the 6502 emulator by
	Juergen Buchmueller.  It is released as part of the Mame emulator project.
	Let me know if you intend to use this code in any other project.


	NOTICE:

	This code is not currently complete!  Several things are unimplemented,
	some due to lack of time, some due to lack of documentation, mainly
	due to lack of programs using these features.

	csh, csl opcodes are not supported.
	set opcode and T flag behaviour are not supported.

	I am unsure if instructions like SBC take an extra cycle when used in
	decimal mode.  I am unsure if flag B is set upon execution of rti.

	Cycle counts should be quite accurate, illegal instructions are assumed
	to take two cycles.

	Internal timer (TIQ) not properly implemented.

******************************************************************************/

#include "driver.h"
#include "../src/memory.h"
#include "cpuintrf.h"
#include "h6280.h"

#include <stdio.h>
extern int cpu_getpc(void);

int 	H6280_ICount = 0;
static	H6280_Regs	h6280;

/* Hardwire zero page memory to Bank 8, only one Hu6280 is supported if this
is selected */
#define FAST_ZERO_PAGE

#ifdef FAST_ZERO_PAGE
unsigned char *ZEROPAGE;
#endif

/* Comment next line to use opcodes table instead of a big switch for opcodes execution ! Franxis 24-06-2006 */
#define H6280_BIG_SWITCH

/* include the macros */
#include "h6280ops.h"

/* include the opcode macros, functions and function pointer tables */
#include "tblh6280.h"

/*****************************************************************************/

unsigned H6280_GetPC (void)
{
	return PCD;
}

void H6280_GetRegs (H6280_Regs *Regs)
{
	*Regs = h6280;
}

void H6280_SetRegs (H6280_Regs *Regs)
{
	h6280 = *Regs;
}

void H6280_Reset(void)
{
	/* wipe out the h6280 structure */
	fast_memset(&h6280, 0, sizeof(H6280_Regs));

	/* set I and Z flags */
	P = _fI | _fZ;

    /* stack starts at 0x01ff */
	h6280.SP.D = 0x1ff;

    /* read the reset vector into PC */
	PCL = RDMEM(H6280_RESET_VEC);
	PCH = RDMEM((H6280_RESET_VEC+1));

    /* clear pending interrupts */
#if NEW_INTERRUPT_SYSTEM
	for (i = 0; i < 3; i++)
		h6280.irq_state[i] = CLEAR_LINE;
	h6280.pending_interrupt = 0;
#else
	H6280_Clear_Pending_Interrupts();
#endif

#ifdef FAST_ZERO_PAGE
{
	extern unsigned char *cpu_bankbase[];
	ZEROPAGE=cpu_bankbase[8];
}
#endif

}

int H6280_Execute(int cycles)
{
	extern int previouspc;
	int in;
	H6280_ICount = cycles;

	/* Execute instructions */
	do
    	{
		previouspc = PCD;

		/* Execute 1 instruction */
		in=RDOP();
		PCW++;
		
		#ifdef H6280_BIG_SWITCH
			H6280_SWITCH
		#else
			insnh6280[in]();
		#endif

		/* Check interrupts */
		if (h6280.pending_interrupt) {
			if (!(P & _fI)) {
				if ((h6280.pending_interrupt&H6280_IRQ2_MASK) && !(h6280.irq_mask&0x2))
				{
					DO_INTERRUPT(H6280_IRQ2_MASK,H6280_IRQ2_VEC);
				}

				if ((h6280.pending_interrupt&H6280_IRQ1_MASK) && !(h6280.irq_mask&0x1))
				{
					DO_INTERRUPT(H6280_IRQ1_MASK,H6280_IRQ1_VEC);
				}

				if ((h6280.pending_interrupt&H6280_TIMER_MASK) && !(h6280.irq_mask&0x4))
				{
					DO_INTERRUPT(H6280_TIMER_MASK,H6280_TIMER_VEC);
				}
			}

			if (h6280.pending_interrupt&H6280_NMI_MASK)
			{
				DO_INTERRUPT(H6280_NMI_MASK,H6280_NMI_VEC);
			}
		}

		/* If PC has not changed we are stuck in a tight loop, may as well finish */
		if (PCD==previouspc) {
			if (H6280_ICount>0) H6280_ICount=0;
		}

	} while (H6280_ICount > 0);

	return cycles - H6280_ICount;
}

/*****************************************************************************/

#ifdef NEW_INTERRUPT_SYSTEM
void H6280_set_nmi_line(int state)
{
	if (h6280.nmi_state == state) return;
	h6280.nmi_state = state;
	if (state != CLEAR_LINE)
        h6280.pending_interrupt |= H6280_INT_NMI;
}

void H6280_set_irq_line(int irqline, int state)
{
    h6280.irq_state[irqline] = state;
	if (state == CLEAR_LINE) {
		switch (irqline) {
			case 0: h6280.pending_interrupt &= ~H6280_IRQ1_MASK; break;
			case 1: h6280.pending_interrupt &= ~H6280_IRQ2_MASK; break;
			case 2: h6280.pending_interrupt &= ~H6280_TIMER_MASK; break;
		}
	} else {
		switch (irqline) {
			case 0: h6280.pending_interrupt |= H6280_IRQ1_MASK; break;
			case 1: h6280.pending_interrupt |= H6280_IRQ2_MASK; break;
			case 2: h6280.pending_interrupt |= H6280_TIMER_MASK; break;
        }
    }
}

void H6280_set_irq_callback(int (*callback)(int irqline))
{
	h6280.irq_callback = callback;
}

#else

void H6280_Cause_Interrupt(int type)
{
    switch (type)
    {
		case H6280_INT_NONE:
            break;
		case H6280_INT_NMI:
			h6280.pending_interrupt |= H6280_NMI_MASK;
			break;
		case H6280_INT_TIMER:
			h6280.pending_interrupt |= H6280_TIMER_MASK;
			break;
		case H6280_INT_IRQ2:
			h6280.pending_interrupt |= H6280_IRQ2_MASK;
			break;
		case H6280_INT_IRQ1:
			h6280.pending_interrupt |= H6280_IRQ1_MASK;
			break;
		default:
            break;
    }
}

void H6280_Clear_Pending_Interrupts(void)
{
	h6280.pending_interrupt=H6280_INT_NONE;
}

#endif

/*****************************************************************************/

int H6280_irq_status_r(int offset)
{
	int status;

	switch (offset) {
		case 0: /* Read irq mask */
			return h6280.irq_mask;

		case 1: /* Read irq status */
			status=0;
			/* Almost certainly wrong... */
/*			if (h6280.pending_irq1) status^=1; */
/*			if (h6280.pending_irq2) status^=2; */
/*			if (h6280.pending_timer) status^=4; */
			return status;
	}

	return 0;
}

void H6280_irq_status_w(int offset, int data)
{
	switch (offset) {
		case 0: /* Write irq mask */
			h6280.irq_mask=data&0x7;
			return;

		case 1: /* Reset timer irq */

/*aka TIQ acknowledge */
/* if not ack'd TIQ cant fire again?! */
/* */
			h6280.timer_value=h6280.timer_load; /* hmm */
			return;
	}
}

int H6280_timer_r(int offset)
{
	switch (offset) {
		case 0: /* Counter value */
			return h6280.timer_value;

		case 1: /* Read counter status */
			return h6280.timer_status;
	}

	return 0;
}

void H6280_timer_w(int offset, int data)
{
	switch (offset) {
		case 0: /* Counter preload */

/*H6280_Cause_Interrupt(H6280_INT_TIMER); */

			h6280.timer_load=h6280.timer_value=data;
			return;

		case 1: /* Counter enable */
			h6280.timer_status=data&1;
			return;
	}
}

/*****************************************************************************/
