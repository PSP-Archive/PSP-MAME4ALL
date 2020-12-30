/***************************************************************************

  majhavoc.c (machine)

  Functions to emulate general aspects of the machine
  (RAM, ROM, interrupts, I/O ports)

***************************************************************************/
#include "driver.h"
#include "vidhrdw/avgdvg.h"
#include "m6502/m6502.h"

int gamma_data;
int alpha_data;
int alpha_rcvd;
int alpha_xmtd;
int gamma_rcvd;
int gamma_xmtd;

int bank_select;
int player_1;
int bank1;

void mhavoc_ram_banksel_w (int offset,int data)
{
	static int bank[2] = { 0x20200, 0x20800 };
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];

	data&=0x01;
	cpu_setbank (1, &RAM[bank[data]]);
}

void mhavoc_rom_banksel_w (int offset,int data)
{
	static int bank[4] = { 0x10000, 0x12000, 0x14000, 0x16000 };
	unsigned char *RAM = Machine->memory_region[Machine->drv->cpu[0].memory_region];


	data &= 0x03;

	cpu_setbank (2, &RAM[bank[data]]);
}

void mhavoc_init_machine (void)
{
	/* Set all the banks to the right place */
	mhavoc_ram_banksel_w (0,0);
	mhavoc_rom_banksel_w (0,0);
	bank_select = -1;
	alpha_data=0;
	gamma_data=0;
	alpha_rcvd=0;
	alpha_xmtd=0;
	gamma_rcvd=0;
	gamma_xmtd=0;
	player_1 = 0;
}

/* Read from the gamma processor */
int mhavoc_gamma_r (int offset)
{
	alpha_rcvd=1;
	gamma_xmtd=0;
	return gamma_data;
}

/* Read from the alpha processor */
int mhavoc_alpha_r (int offset)
{
	gamma_rcvd=1;
	alpha_xmtd=0;
	return alpha_data;
}

/* Write to the gamma processor */
void mhavoc_gamma_w (int offset, int data)
{
	gamma_rcvd=0;
	alpha_xmtd=1;
	alpha_data = data;
	cpu_cause_interrupt (1, M6502_INT_NMI);

	/* the sound CPU needs to reply in 250ms (according to Neil Bradley) */
	timer_set (TIME_IN_USEC(250), 0, 0);
}

/* Write to the alpha processor */
void mhavoc_alpha_w (int offset, int data)
{
	alpha_rcvd=0;
	gamma_xmtd=1;
	gamma_data = data;
}

/* Simulates frequency and vector halt */
int mhavoc_port_0_r(int offset)
{
	int res;

	res = readinputport(0);
	if (player_1)
		res = (res & 0x3f) | (readinputport (5) & 0xc0);

	/* Emulate the 2.4Khz source on bit 2 (divide 2.5Mhz by 1024) */
	if (cpu_gettotalcycles() & 0x400)
		res &=~0x02;
	else
		res|=0x02;

	if (avgdvg_done())
		res |=0x01;
	else
		res &=~0x01;

	if (gamma_rcvd==1)
		res |=0x08;
	else
		res &=~0x08;

	if (gamma_xmtd==1)
		res |=0x04;
	else
		res &=~0x04;

	return (res & 0xff);
}

int mhavoc_port_1_r(int offset)
{
	int res;

	res=readinputport(1);

	if (alpha_rcvd==1)
		res |=0x02;
	else
		res &=~0x02;

	if (alpha_xmtd==1)
		res |=0x01;
	else
		res &=~0x01;

	return (res & 0xff);
}

void mhavoc_out_0_w (int offset, int data)
{
	if (!(data & 0x08))
	{
		cpu_reset(1);
		alpha_rcvd=0;
		alpha_xmtd=0;
		gamma_rcvd=0;
		gamma_xmtd=0;
	}
	player_1 = data & 0x20;
	/* Emulate the roller light (Blinks on fatal errors) */
	osd_led_w (2, data & 0x01);
}

void mhavoc_out_1_w (int offset, int data)
{
	osd_led_w (1, data & 0x01);
	osd_led_w (0, (data & 0x02)>>1);
}

int mhavoc_gammaram_r (int offset)
{
	extern unsigned char *RAM;


	return RAM[offset & 0x7ff];
}

void mhavoc_gammaram_w (int offset, int data)
{
	extern unsigned char *RAM;


	RAM[offset & 0x7ff] = data;
}
